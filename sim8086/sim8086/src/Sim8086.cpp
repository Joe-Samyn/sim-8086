
#include "Sim8086.h"
#include "Execute.h"
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

    if (ex.calculationType == Effective_addr_direct_address) {
        physicalAddress = { .segment=cpu.segmentRegisters[DS], .offset=(uint16_t)ex.displacement };
    }
    else {
        uint16_t logicalAddr = cpu.registers[ex.base.index] + cpu.registers[ex.index.index] + ex.displacement;
        physicalAddress.offset = logicalAddr;
    }

    return physicalAddress;
}

void WriteDataToOperand(CPU &cpu, const Operand &op, uint16_t data, uint8_t size) {
    if (op.type == OpType_register) {
        WriteToRegister(cpu, op.reg, data);
    }
    else if (op.type == OpType_effectiveAddrCalc) {
        SegmentedAddress physicalAddress = ComputeEffectiveAddress(cpu, op.expression);
        if (size == WIDE) {
            WriteWordToMemory(data, physicalAddress);
        } else {
            WriteByteToMemory(data, physicalAddress);
        }
    }
}

uint16_t ExtractDataFromOperand(CPU &cpu, Operand src, uint8_t size) {

    uint16_t value = 0;
    if (src.type == OpType_immediate) {
        value = src.immediate;
    } 
    else if (src.type == OpType_effectiveAddrCalc) {
        SegmentedAddress physicalAddress = ComputeEffectiveAddress(cpu, src.expression);
        value = size == WIDE ? ReadWordFromMemory(physicalAddress) : ReadByteFromMemory(physicalAddress);
    } 
    else if (src.type == OpType_register) {
        value = ReadFromRegister(cpu, src.reg);
    }

    return value;
}

void ExecuteMov(CPU &cpu, const Operand &src, const Operand &dest, uint8_t size) {
    uint16_t v0 = ExtractDataFromOperand(cpu, src, size);
    WriteDataToOperand(cpu, dest, v0, size);
}

void ExecuteAdd(CPU &cpu, Operand src, Operand dest, uint8_t size, bool useCarry) {
    uint16_t v0 = ExtractDataFromOperand(cpu, src, size);
    uint16_t v1 = ExtractDataFromOperand(cpu, dest, size);
    bool carry = useCarry && (cpu.flags & Carry);
    uint16_t result = v0 + v1 + carry;
    ComputeOF(cpu, (int16_t)v0, (int16_t)v1, (int16_t)result, size);
    ComputeSF(cpu, (int16_t)result, size);
    ComputeZF(cpu, result, size);
    ComputeCF(cpu, v0, v1, result, size);
    WriteDataToOperand(cpu, dest, result, size);
}

void ExecuteSub(CPU &cpu, Operand src, Operand dest, uint8_t size, bool useCarry) {
    uint16_t v0 = ExtractDataFromOperand(cpu, dest, size);
    uint16_t v1 = ExtractDataFromOperand(cpu, src, size);

    bool carry = useCarry && (cpu.flags & Carry);
    uint16_t result = v0 - v1 - carry;
    WriteDataToOperand(cpu, dest, result, size);

    // NOTE: v1 is inverted because dest - src == dest + twos_complement(src) in 8086
    ComputeCF(cpu, -v1, v0, result, size, true);
    ComputeOF(cpu, -v1, v0, result, size);
    ComputeSF(cpu, result, size);
    ComputeZF(cpu, result, size);
}

void ExecuteCmp(CPU &cpu, Operand src, Operand dest, uint8_t size) {
    uint16_t v0 = ExtractDataFromOperand(cpu, dest, size);
    uint16_t v1 = ExtractDataFromOperand(cpu, src, size);

    uint16_t result = v0 - v1;

    ComputeCF(cpu, -v1, v0, result, size, true);
    ComputeOF(cpu, -v1, v0, result, size);
    ComputeSF(cpu, result, size);
    ComputeZF(cpu, result, size);
}

void Execute(Program &program)
{
    CPU cpu = {};
    DisplayRegisterState(cpu);
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
                    WriteInstructionToOutput(result, Console);

                    switch(result.op)
                    {
                        case Op_count: break;
                        case Op_MOV:
                        {
                            uint8_t size = (result.flags & Wide);
                            ExecuteMov(cpu, result.operands[SRC], result.operands[DEST], size);
                        } break;
                        case Op_ADD:
                        {
                            ExecuteAdd(cpu, result.operands[SRC], result.operands[DEST], (result.flags & Wide));
                        } break;
                        case Op_ADC:
                        {
                            ExecuteAdd(cpu, result.operands[SRC], result.operands[DEST], (result.flags & Wide), true);
                        } break;
                        case Op_SUB:
                        {
                            ExecuteSub(cpu, result.operands[SRC], result.operands[DEST], (result.flags & Wide));
                        } break;
                        case Op_SBB:
                        {
                            ExecuteSub(cpu, result.operands[SRC], result.operands[DEST], (result.flags & Wide), true);
                        } break;
                        case Op_CMP:
                        {
                            ExecuteCmp(cpu, result.operands[SRC], result.operands[DEST], (result.flags & Wide));
                        } break;
                    }


                    cpu.IP = at.offset;
                    break;
                }
            }

        }
    }
    printf("\n");
    DisplayCpuFlagState(cpu);
    DisplayRegisterState(cpu);
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
                    cpu.IP = at.offset;
                    WriteInstructionToOutput(result, File);
                    break;
                }
            }

        }
    }
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
