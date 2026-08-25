#include "IO.h"
#include "Sim8086.h"

#include <format>
#include <cstdio>

std::FILE* outFile; 

void OpenAsmFile(std::string name)
{
    outFile = std::fopen(name.c_str(), "w");

    // TODO: Check if file failed to open.
    if (!outFile)
    {
        std::cerr << "Could not open file" << std::endl;
    }

    // Write header of asm file 
    std::fprintf(outFile, "bits 16\n\n");
}

void CloseAsmFile()
{
    std::fclose(outFile);
}

void PrintEffectiveAddressExpression(Operand op)
{
    switch(op.expression.calculationType)
    {
        case Effective_addr_direct_address:
            {
                printf("[%d]", op.expression.displacement); 
            } break;
        case Effective_addr_bx_si:
        case Effective_addr_bx_di:
        case Effective_addr_bp_si:
        case Effective_addr_bp_di:
            {
                const char* base = RegisterNames[op.expression.base.index][op.expression.base.offset];
                const char* index = RegisterNames[op.expression.index.index][op.expression.index.offset];
                if (op.expression.hasDisplacement == FALSE)
                {
                    printf("[%s + %s]", base, index);
                }
                else
                {   
                    if (op.expression.displacement < 0)
                    {
                        printf("[%s + %s - %d]", base, index, -op.expression.displacement);
                    }
                    else
                    {
                        printf("[%s + %s + %d]", base, index, op.expression.displacement);
                    }
                }
            } break;
        case Effective_addr_si:
        case Effective_addr_di:
        case Effective_addr_bx:
        case Effective_addr_bp:
            {
                const char* base = RegisterNames[op.expression.base.index][op.expression.base.offset];
                if (op.expression.displacement == 0)
                {
                    printf("[%s]", base);
                }
                else
                {
                    if (op.expression.displacement < 0)
                    {
                        printf("[%s - %d]", base, -op.expression.displacement);
                    }
                    else 
                    {
                        printf("[%s + %d]", base, op.expression.displacement);
                    }
                }
            } break;
            case Effective_addr_count:
            { 
            } break;
    }
}

void WriteEffectiveAddressToFile(Operand op)
{
    switch(op.expression.calculationType)
    {
        case Effective_addr_direct_address:
            {
                std::fprintf(outFile, "[%d]", op.expression.displacement); 
            } break;
        case Effective_addr_bx_si:
        case Effective_addr_bx_di:
        case Effective_addr_bp_si:
        case Effective_addr_bp_di:
            {
                const char* base = RegisterNames[op.expression.base.index][op.expression.base.offset];
                const char* index = RegisterNames[op.expression.index.index][op.expression.index.offset];
                if (op.expression.hasDisplacement == FALSE)
                {
                    std::fprintf(outFile, "[%s + %s]", base, index);
                }
                else
                {   
                    if (op.expression.displacement < 0)
                    {
                        std::fprintf(outFile, "[%s + %s - %d]", base, index, -op.expression.displacement);
                    }
                    else
                    {
                        std::fprintf(outFile, "[%s + %s + %d]", base, index, op.expression.displacement);
                    }
                }
            } break;
        case Effective_addr_si:
        case Effective_addr_di:
        case Effective_addr_bx:
        case Effective_addr_bp:
            {
                const char* base = RegisterNames[op.expression.base.index][op.expression.base.offset];
                if (op.expression.displacement == 0)
                {
                    std::fprintf(outFile, "[%s]", base);
                }
                else
                {
                    if (op.expression.displacement < 0)
                    {
                        std::fprintf(outFile, "[%s - %d]", base, -op.expression.displacement);
                    }
                    else 
                    {
                        std::fprintf(outFile, "[%s + %d]", base, op.expression.displacement);
                    }
                }
            } break;
            case Effective_addr_count:
            { 
            } break;
    }
}

void PrintOperand(Operand op)
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
                PrintEffectiveAddressExpression(op);
            } break;
        case OpType_immediate:
        {
            printf("%d", op.immediate);
        } break;
        case OpType_jmp:
        {
            printf("$%+d", op.address);
            
        } break;
        default:
            {

            }
    }
}

void WriteOperandToFile(Operand op)
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
                WriteEffectiveAddressToFile(op);
            } break;
        case OpType_immediate:
        {
            std::fprintf(outFile, "%d", op.immediate);
        } break;
        case OpType_jmp:
        {
            std::fprintf(outFile, "$%+d", op.address);
            
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
    WriteOperandToFile(instruction.operands[1]);

    if (instruction.operands[0].type != OpType_none)
    {
        std::fprintf(outFile, ", ");
    }   

    // Print src operand 
    WriteOperandToFile(instruction.operands[0]);

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
    PrintOperand(instruction.operands[1]);

    if (instruction.operands[0].type != OpType_none)
    {
        printf(", ");
    }   

    // Print src operand 
    PrintOperand(instruction.operands[0]);

    printf("\n");
}

void DisplayRegisterState(CPU cpu)
{
    printf("Register State\n");
    for (int i = 0; i < Register_count; i++)
    {
        printf("%s   0x%04X\n", RegisterNames[i][2], cpu.registers[i]);
    }

    printf("\n");
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
}

void WriteInstructionToOutput(const Instruction &instruction, uint8_t outputLocation) {
    if (outputLocation & File) {
        WriteToFile(instruction);
    }

    if (outputLocation & Console) {
        WriteToConsole(instruction);
    }
}