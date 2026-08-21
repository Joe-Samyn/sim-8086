
#include "Sim8086.h"
#include "Decode.h"
#include "IO.h"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <cstdio>



#define ArrayCount(array) sizeof(array)/sizeof(array[0])

// NOTE - Memory is stored little endian in this simulator
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

void WriteWordToMemory(uint16_t value, SegmentedAddress at) {
    uint32_t physicalAddress = ComputePhysicalAddress(at);
    uint8_t lowByte = GetLowByte(value);
    uint8_t hiByte = GetHiByte(value);
    Memory[physicalAddress] = lowByte;
    Memory[physicalAddress+1] = hiByte;
}

void WriteByteToMemory(uint16_t value, SegmentedAddress at) {
    uint32_t physicalAddress = ComputePhysicalAddress(at);
    Memory[physicalAddress] = value;
}

SegmentedAddress Create(uint16_t segment, uint16_t offset) {
    return { 
        .segment=segment, 
        .offset=(uint16_t)offset 
    };
}

void ComputeOF(CPU &cpu, int16_t src, int16_t dest, int16_t result, uint8_t size){
    if (size == BYTE) {
        int8_t bSrc = (int8_t)src;
        int8_t bDest = (int8_t)dest;
        int8_t bResult = (int8_t) result;
        if ((bSrc < 0 && bDest < 0 && bResult >= 0) || (bSrc > 0 & bDest > 0 && bResult <= 0)) {
            cpu.flags |= Overflow;
        }
        else {
            cpu.flags &= ~Overflow;
        }
    }
    else {
        if ((src < 0 && dest < 0 && result >= 0) || (src > 0 & dest > 0 && result <= 0)) {
            cpu.flags |= Overflow;
        }
        else {
            cpu.flags &= ~Overflow;
        }
    }
}

void ComputeSF(CPU &cpu, int16_t result, uint8_t size) {
    if (size == BYTE) {
        int8_t bResult = (int8_t)result;
        if (bResult < 0) {
            cpu.flags |= Sign;
        } 
        else {
            cpu.flags &= ~Sign;
        }
    }
    else {
        if (result < 0) {
            cpu.flags |= Sign;
        } 
        else {
            cpu.flags &= ~Sign;
        }
    }
}

void ComputeZF(CPU &cpu, uint16_t result, uint8_t size) {
    if (size == BYTE) {
        if ((int8_t)result == 0) {
            cpu.flags |= Zero;
        }
        else {
            cpu.flags &= ~Zero;
        }
    } 
    else {
        if (result == 0) {
            cpu.flags |= Zero;
        }
        else {
            cpu.flags &= ~Zero;
        }
    }
}

void ComputeCF(CPU &cpu, uint16_t src, uint16_t dest, uint16_t result, uint8_t size, bool invert) {
    if (size == BYTE) {
        uint8_t bSrc = (uint8_t)src;
        uint8_t bDest = (uint8_t)dest;
        uint8_t bResult = (uint8_t) result;
        if (invert) {
            if (bResult < bSrc && bResult < bDest ) {
                cpu.flags &= ~Carry;
            }
            else {
                cpu.flags |= Carry;
            }
        }
        else {
            if (bResult < bSrc && bResult < bDest ) {
                cpu.flags |= Carry;
            }
            else {
                cpu.flags &= ~Carry;
            }
        }
    }
    else {
        if (invert) {
            if (result < src && result < dest ) {
                cpu.flags &= ~Carry;
            }
            else {
                cpu.flags |= Carry;
            }
        }
        else {
            if (result < src && result < dest ) {
                cpu.flags |= Carry;
            }
            else {
                cpu.flags &= ~Carry;
            }
        }
    }
    
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

void WriteToRegister(CPU &cpu, RegisterAccess ra, uint16_t data) {

    if (ra.offset == LO_BITS)
    {
        cpu.registers[ra.index] = WriteLoByte(cpu.registers[ra.index], data);
    }
    else if (ra.offset == HI_BITS)
    {
        cpu.registers[ra.index] = WriteHiByte(cpu.registers[ra.index], data);
    }
    else
    {
        cpu.registers[ra.index] = data;
    }
}

/**
* @brief Computes the physical segmented address represented by an effective address expression
*/
SegmentedAddress ComputeEffectiveAddress(CPU cpu, EffectiveAddrExpression ex) {
    SegmentedAddress physicalAddress = {
        .segment=cpu.segmentRegisters[DS]
    };

    switch(ex.calculationType) {
        case Effective_addr_count: break;
        case Effective_addr_direct_address:
        {
            physicalAddress = { .segment=cpu.segmentRegisters[DS], .offset=(uint16_t)ex.displacement };
        } break;
        case Effective_addr_bx:
        {
            uint16_t logicalAddr = cpu.registers[Register_b] + ex.displacement;
            physicalAddress.offset = logicalAddr;
        } break;
        case Effective_addr_bp:
        {
            uint16_t logicalAddr = cpu.registers[Register_bp] + ex.displacement;
            physicalAddress.offset = logicalAddr;
        } break;
        case Effective_addr_di:
        {
            uint16_t logicalAddr = cpu.registers[Register_di] + ex.displacement;
            physicalAddress.offset = logicalAddr;
        } break;
        case Effective_addr_si:
        {
            uint16_t logicalAddr = cpu.registers[Register_si] + ex.displacement;
            physicalAddress.offset = logicalAddr;
        } break;
        case Effective_addr_bx_si:
        {
            uint16_t logicalAddr = cpu.registers[Register_b] + cpu.registers[Register_si] + ex.displacement;
            physicalAddress.offset = logicalAddr;
        } break;
        case Effective_addr_bx_di:
        {
            uint16_t logicalAddr = cpu.registers[Register_b] + cpu.registers[Register_di] + ex.displacement;
            physicalAddress.offset = logicalAddr;
        } break;
        case Effective_addr_bp_di:
        {
            uint16_t logicalAddr = cpu.registers[Register_bp] + cpu.registers[Register_di] + ex.displacement;
            physicalAddress.offset = logicalAddr;
        } break;
        case Effective_addr_bp_si:
        {
            uint16_t logicalAddr = cpu.registers[Register_bp] + cpu.registers[Register_si] + ex.displacement;
            physicalAddress.offset = logicalAddr;
        } break;
    }

    return physicalAddress;
}

uint16_t ExtractDataFromSrcOperand(CPU &cpu, Operand src, uint8_t size) {
    uint16_t srcData = 0;
    if (src.type == OpType_immediate) {
        srcData = src.immediate;
    } 
    else if (src.type == OpType_effectiveAddrCalc) {
        SegmentedAddress physicalAddress = ComputeEffectiveAddress(cpu, src.expression);
        srcData = size == WIDE ? ReadWordFromMemory(physicalAddress) : ReadByteFromMemory(physicalAddress);
    } else if (src.type == OpType_register) {
        srcData = ReadFromRegister(cpu, src.reg);
    }

    return srcData;
}

void ExecuteMov(CPU &cpu, Operand src, Operand dest, uint8_t size) 
{
    // TODO - Make this an if-else chain because all cases will never be handled here 
    // What src are we dealing with? Get the value that needs to be moved into dest 
    uint16_t srcData = ExtractDataFromSrcOperand(cpu, src, size);

    // Determine destination type
    if (dest.type == OpType_register)
    {
        // TODO - Print statements should only be in the IO files. This needs to get refactored into the IO files so that RegisterNames
        // can be moved to IO.cpp properly
        printf("%s <-- 0x%04X\n\n", RegisterNames[dest.reg.index][dest.reg.offset], srcData);
        RegisterAccess ra = dest.reg;
        WriteToRegister(cpu, ra, srcData);
    } 
    else if (dest.type == OpType_effectiveAddrCalc)
    {
        SegmentedAddress physicalAddress = ComputeEffectiveAddress(cpu, dest.expression);
        if (size == WIDE) WriteWordToMemory(srcData, physicalAddress);
        else WriteByteToMemory(srcData, physicalAddress);
    }
}

void ExecuteAdd(CPU &cpu, Operand src, Operand dest, uint8_t size) {

    uint16_t srcData = ExtractDataFromSrcOperand(cpu, src, size);
    uint16_t destData = 0;
    uint16_t result = 0;
    if (dest.type == OpType_register)
    {
        destData = ReadFromRegister(cpu, dest.reg);
        printf("%s <-- 0x%04X + 0x%04X\n\n", RegisterNames[dest.reg.index][dest.reg.offset], destData, srcData); // TODO - Print statements should only be in the IO files. This needs to get refactored into the IO files so that RegisterNames can be moved to IO.cpp properly
        result = destData + srcData;
        WriteToRegister(cpu, dest.reg, result);
    } 
    else if (dest.type == OpType_effectiveAddrCalc)
    {
        SegmentedAddress physicalAddress = ComputeEffectiveAddress(cpu, dest.expression);
        if (size == WIDE) {
            destData = ReadWordFromMemory(physicalAddress);
            result = destData + srcData;
            WriteWordToMemory(result, physicalAddress);
        } else {
            destData = ReadByteFromMemory(physicalAddress);
            result = destData + srcData;
            WriteByteToMemory(result, physicalAddress);
        }
    }

    ComputeOF(cpu, (int16_t)srcData, (int16_t)destData, (int16_t)result, size);
    ComputeSF(cpu, (int16_t)result, size);
    ComputeZF(cpu, result, size);
    ComputeCF(cpu, srcData, destData, result, size);
    DisplayCpuFlagState(cpu);
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
                        case Op_count: break;
                        case Op_MOV:
                        {
                            ExecuteMov(cpu, result.operands[SRC], result.operands[DEST], (result.flags & Wide));
                        } break;
                        case Op_ADD:
                        {
                            ExecuteAdd(cpu, result.operands[SRC], result.operands[DEST], (result.flags & Wide));
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
