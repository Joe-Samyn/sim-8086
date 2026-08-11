
#include "Sim8086.h"
#include "Decode.h"
#include "IO.h"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <cstdio>



#define ArrayCount(array) sizeof(array)/sizeof(array[0])

#define MEMORY_SIZE 1024 * 1024

static uint8_t Memory[MEMORY_SIZE];


uint8_t ReadByteFromMemory(SegmentedAddress at) {
    uint32_t address = ComputePhysicalAddress(at);
    return Memory[address];
}

uint16_t ReadWordFromMemory(SegmentedAddress at) {
    uint32_t address = ComputePhysicalAddress(at);
    uint8_t lo = Memory[address];
    uint16_t hi = Memory[address+1];
    return ((hi << 8) | lo);
}

uint8_t FetchNextInstructionByte(CPU &cpu) {
    SegmentedAddress at = { .segment=cpu.segmentRegisters[CS], .offset=cpu.IP};
    uint32_t address = ComputePhysicalAddress(at);
    cpu.IP++;
    return Memory[address];
}

SegmentedAddress Create(uint16_t segment, uint16_t offset) {
    return { 
        .segment=segment, 
        .offset=(uint16_t)offset 
    };
}

Entry InstructionTable[] = {
#include "InstructionTable.inl"
};

uint16_t ReadFromRegister(CPU cpu, RegisterAccess ra)
{
    uint16_t result = 0;
    switch(ra.offset)
    {
        case LO_BITS:
            result = ReadLoByte(cpu.registers[ra.index]);
            break;
        case HI_BITS:
            result = ReadHiByte(cpu.registers[ra.index]);
            break;
        case FULL_BITS:
            result = cpu.registers[ra.index];
            break;
    }

    return result;
}

void ExecuteMov(CPU &cpu, Operand src, Operand dest) 
{
    // What src are we dealing with? Get the value that needs to be moved into dest 
    uint16_t srcData = 0;
    switch(src.type)
    {
        case OpType_immediate:
            srcData = src.immediate;
            break;
        case OpType_effectiveAddrCalc:
            // TODO - Caclulate address or use direct address
            break;
        case OpType_register:
        {   
            srcData = ReadFromRegister(cpu, src.reg);
        } break;
    }

    // Determine destination type
    if (dest.type == OpType_register)
    {
        // TODO - Print statements should only be in the IO files. This needs to get refactored into the IO files so that RegisterNames
        // can be moved to IO.cpp properly
        printf("%s <-- 0x%04X\n\n", RegisterNames[dest.reg.index][dest.reg.offset], srcData);

        RegisterAccess ra = dest.reg;
        if (ra.offset == LO_BITS)
        {
            cpu.registers[ra.index] = WriteLoByte(cpu.registers[ra.index], srcData);
        }
        else if (ra.offset == HI_BITS)
        {
            cpu.registers[ra.index] = WriteHiByte(cpu.registers[ra.index], srcData);
        }
        else
        {
            cpu.registers[dest.reg.index] = srcData;
        }
    } 
    else if (dest.type == OpType_effectiveAddrCalc)
    {
        // TODO - Calculate address and write value into memory address
    }
}

// TODO - Execute and Disassembly utilize the exact same loop to iterate over instructions. This needs to be moved into the 
// Decode files and reused appropriately. 
void Execute(Program &program)
{
    CPU cpu = {};
    while (cpu.IP <= program.endAddr) 
    {
        uint8_t currentByte = FetchNextInstructionByte(cpu);
        Entry entry = {};

        DisplayRegisterState(cpu);

        // Search Instruction table for matching instruction 
        for (int i = 0; i < ArrayCount(InstructionTable); i++)
        {
            entry = InstructionTable[i];

            if (entry.bits[0].value == (currentByte >> (8 - entry.bits[0].count)))
            {
                SegmentedAddress at = Create(cpu.segmentRegisters[CS], cpu.IP - 1);
                Instruction result = Decode(entry, at);
                if (result.op)
                {
                    WriteToConsole(result);

                    switch(result.op)
                    {
                        case Op_MOV:
                        {
                            ExecuteMov(cpu, result.operands[SRC], result.operands[DEST]);
                        } break;
                    }


                    cpu.IP = at.offset;
                    break;
                }
            }

        }

        DisplayRegisterState(cpu);
    }
}

void Disassemble(Program &program)
{	
    CPU cpu = { 0 };

    while (cpu.IP <= program.endAddr)
    {
        uint8_t currentByte = FetchNextInstructionByte(cpu);
        Entry entry = {};

        // Search Instruction table for matching instruction 
        for (int i = 0; i < ArrayCount(InstructionTable); i++)
        {
            entry = InstructionTable[i];

            if (entry.bits[0].value == (currentByte >> (8 - entry.bits[0].count)))
            {
                SegmentedAddress at = Create(cpu.segmentRegisters[CS], cpu.IP - 1);
                Instruction result = Decode(entry, at);
                if (result.op)
                {
                    DecodedInstructions[DecodedInstIndex] = result;
                    DecodedInstIndex++;
                    cpu.IP = at.offset;
                    break;
                }
            }

        }
    }

    WriteToConsole();
}

Program LoadProgramIntoMemory(std::string filePath)
{
    // Open binary file, at the end (ate)
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);

    if (!file.is_open() || errno == ENOENT)
    {
        std::cerr << "ERROR: Could not open file. File does not exist.\n";
        return { 0 };
    }

    // Get file size
    uint32_t length = static_cast<uint32_t>(file.tellg());
    file.seekg(0, file.beg);
    file.read(reinterpret_cast<char*>(Memory), length);

    Program program = {
        .size=length,
        .startAddr=0,
        .endAddr=length - 1
    };
    return program; 
}
