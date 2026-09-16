#include "IO.h"
#include "Sim8086.h"

#include <cstdio>

/**
 * TODO: IO was not a first class citizen of this project. It was a means to validate functionality until tests
 * were put in place. It needs to be revisted now that we have a working disassembler and execution pipeline and
 * built properly.
 *
 * Things to determine:
 *  - How does console/file IO work?
 *  - How do we want to display CPU state?
 *  - How do we enable/disable output levels? Does everything get output all the time when on?
 *  - Can we have different verbosity levels for output?
 *  - What happens when we have a UI wrapping this? How do we output to the UI?
 *  - How do we handle Memory dumps?
 */

std::FILE* outFile;
const char* MemoryFile = "/Users/joey/Projects/sim-8086/sim8086/sim8086/tests/memory_out.data";

void OpenAsmFile(std::string name)
{
    outFile = std::fopen(name.c_str(), "w");

    // TODO: Check if file failed to open.
    if (!outFile)
    {
        int err = errno;
        printf("ERROR::Could not open file. CODE: %d\n", err);
    }

    // Write header of asm file
    std::fprintf(outFile, "bits 16\n\n");
}

void CloseAsmFile()
{
    std::fclose(outFile);
}

void PrintEAExpressionToConsole(const Operand &op) {
    if (op.ea == Direct_address)
    {
        fprintf(outFile, "[%d]", op.displacement);
    }
    else if (op.displacement != 0)
    {
        printf("[%s%+d]", EAExpressions[op.ea], op.displacement);
    }
    else
    {
        printf("[%s]", EAExpressions[op.ea]);
    }
}

void PrintEAExpressionToFile(const Operand &op) {
    if (op.ea == Direct_address)
    {
        fprintf(outFile, "[%d]", op.displacement);
    }
    else if (op.displacement != 0)
    {
        fprintf(outFile, "[%s%+d]", EAExpressions[op.ea], op.displacement);
    }
    else
    {
        fprintf(outFile, "[%s]", EAExpressions[op.ea]);
    }
}

void PrintOperand(Operand op, uint8_t instSize)
{
    switch(op.type)
    {
        case OpType_none:
        {
            return;
        } break;
        case OpType_register:
            {
                const char* name = RegisterNames[op.reg.index][op.reg.offset];
                printf("%s", name);
            } break;
        case OpType_effectiveAddrCalc:
            {
                PrintEAExpressionToConsole(op);
            } break;
        case OpType_immediate:
        {
            printf("0x%04X (%d)", (uint16_t)op.immediate, op.immediate);
        } break;
        case OpType_jmp:
        {
            printf("$%+d", (int16_t)(op.displacement + instSize));

        } break;
        default:
            {

            }
    }
}

void WriteOperandToFile(Operand op, uint8_t instSize)
{
    switch(op.type)
    {
        case OpType_none:
        {
            return;
        } break;
        case OpType_register:
            {
                const char* name = RegisterNames[op.reg.index][op.reg.offset];
                std::fprintf(outFile, "%s", name);
            } break;
        case OpType_effectiveAddrCalc:
            {
                PrintEAExpressionToFile(op);
            } break;
        case OpType_immediate:
        {
            std::fprintf(outFile, "0x%04X", (uint16_t)op.immediate);
        } break;
        case OpType_jmp:
        {
            std::fprintf(outFile, "$%+d", (int16_t)(op.displacement + instSize));

        } break;
        default:
            {

            }
    }
}

void WriteToFile(const Instruction &instruction)
{
    // Print mnemonic/operation
    std::fprintf(outFile, "%s ", Mnemonics[instruction.op]);

    // If either operand type is immediate, we should print size
    if ((instruction.operands[SRC].type == OpType_immediate || instruction.operands[SRC].type == OpType_none) && instruction.operands[DEST].type == OpType_effectiveAddrCalc)
    {
        std::fprintf(outFile, "%s ", (instruction.flags & Flags::Wide) == 0 ? "byte" : "word");
    }

    // Print dest operand
    WriteOperandToFile(instruction.operands[1], instruction.size);

    if (instruction.operands[0].type != OpType_none)
    {
        std::fprintf(outFile, ", ");
    }

    // Print src operand
    WriteOperandToFile(instruction.operands[0], instruction.size);

    std::fprintf(outFile, "\n");
}

void WriteToConsole(const Instruction &instruction) {

    // Print mnemonic/operation
    printf("%s ", Mnemonics[instruction.op]);

    // If either operand type is immediate, we should print size
    if ((instruction.operands[SRC].type == OpType_immediate || instruction.operands[SRC].type == OpType_none) && instruction.operands[DEST].type == OpType_effectiveAddrCalc)
    {
        printf("%s ", (instruction.flags & Flags::Wide) == 0 ? "byte" : "word");
    }

    // Print dest operand
    PrintOperand(instruction.operands[1], instruction.size);

    if (instruction.operands[0].type != OpType_none)
    {
        printf(", ");
    }

    // Print src operand
    PrintOperand(instruction.operands[0], instruction.size);

    printf("\n");
}

void DisplayRegisterState(CPU cpu)
{
    printf("Register State\n");
    for (int i = 0; i < Register_count; i++)
    {
        printf("%s   0x%04X (%d)\n", RegisterNames[i][2], cpu.registers[i], cpu.registers[i]);
    }

    printf("\n");
}

void DisplayCpuFlagState(const CPU &cpu) {
    printf("OF  DF  IF  TF  SF  ZF  AF  PF  CF\n");
    printf("%d   %d   %d   %d   %d   %d   %d   %d   %d\n",
        (cpu.flags & Overflow) > 0,
        (cpu.flags & Direction) > 0,
        (cpu.flags & Interrupt) > 0,
        (cpu.flags & Trap) > 0,
        (cpu.flags & Sign) > 0,
        (cpu.flags & Zero) > 0,
        (cpu.flags & AuxCarry) > 0,
        (cpu.flags & Parity) > 0,
        (cpu.flags & Carry) > 0);
    printf("\n");
}

void WriteInstructionToOutput(const Instruction &instruction, uint8_t outputLocation) {
    if (outputLocation & File) {
        WriteToFile(instruction);
    }

    if (outputLocation & Console) {
        WriteToConsole(instruction);
    }
}

void WriteMemoryToFile(uint8_t *memory) {
    uint32_t size = 1024 * 1024;

    std::FILE* memoryFile = std::fopen(MemoryFile, "wb");

    // TODO: Check if file failed to open.
    if (!memoryFile)
    {
        int err = errno;
        printf("ERROR::Could not open file. CODE: %d\n", err);
    }

    // Write header of asm file
    fwrite(memory, 1, size, memoryFile);
}
