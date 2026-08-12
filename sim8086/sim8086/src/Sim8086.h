#pragma once 

#include <cstdint>

#define LO_BITS 0
#define HI_BITS 1
#define FULL_BITS 2

#define HI_MASK 0xFF00
#define LO_MASK 0x00FF

#define GetLowByte(data) (data & LO_MASK)
#define GetHiByte(data) ((data & HI_MASK) >> 8)

#define WriteLoByte(regValue, data) ((regValue & HI_MASK) | (data & LO_MASK))
#define WriteHiByte(regValue, data) ((data << 8) | (regValue & LO_MASK))

#define ReadLoByte(regValue) (regValue & LO_MASK)
#define ReadHiByte(regValue) ((regValue & HI_MASK) >> 8)

#define TRUE 1
#define FALSE 0

#define SRC 0
#define DEST 1

#define HasField(mask, field) (mask & (1 << field))
#define IncrementAddress(at) at.offset++
#define ComputePhysicalAddress(at) ((at.segment * 16) + at.offset)


struct SegmentedAddress {
    uint16_t segment;
    uint16_t offset;
};

enum RegisterIndex {
    
    Register_a,
    Register_b,
    Register_c,
    Register_d,
    Register_sp,
    Register_bp,
    Register_si,
    Register_di,

    Register_count
};

enum SegmentRegisters {
    CS,
    SS,
    DS,
    ES,

    Segment_count
};

struct CPU {
    uint16_t IP;
    uint16_t registers[Register_count];
    uint16_t segmentRegisters[Segment_count];
};

struct Program {
    uint32_t size;
    uint32_t startAddr;
    uint32_t endAddr;
};

enum Field : uint8_t
{
    Op,
    OpExtension,
    W_bit,
    D_bit,
    Reg_bit,
    Rm_bit,
    Mod_bit,
    Imm_bit,
    Addr_bit,
    S_bit,
    Data_bit,
    Displacement_bit,

    Field_count
};

struct RegisterAccess {
    uint8_t index;      // index of the register in the 8086 manual. For example, register AX/AL is 000 while register CX/CL is 001
    uint8_t offset;     // offset in the register, 0 - low bits, 1 - high bits, 2 - full 16 bits (no offset)
};

enum ModCategory: uint8_t 
{
    Memory_mode_no_disp,
    Memory_mode_8_bit_disp,
    Memory_mode_16_bit_disp,
    Register_mode,

    Mod_category_count
};

enum EffectiveAddressCalculation: uint8_t 
{
    Effective_addr_direct_address,

    Effective_addr_bx_si,
    Effective_addr_bx_di,
    Effective_addr_bp_si,
    Effective_addr_bp_di,
    Effective_addr_si,
    Effective_addr_di,
    Effective_addr_bx,
    Effective_addr_bp,

    Effective_addr_count
};

struct EffectiveAddrExpression
{
    EffectiveAddressCalculation calculationType;
    RegisterAccess base;
    RegisterAccess index;
    uint8_t hasDisplacement;
    int16_t displacement;
};

enum Operation: uint8_t {
    None,
#define INST(mnemonic, ...) Op_##mnemonic,
#define INST_ALT(...)
#include "InstructionTable.inl"
#undef INST
#undef INST_ALT
    Op_count
};

const char* Mnemonics[] = {
    "none",
#define INST(mnemonic, ...) #mnemonic,
#define INST_ALT(...)
#include "InstructionTable.inl"
#undef INST
#undef INST_ALT
};

struct Bits 
{
    Field field;
    uint8_t value;
    uint8_t mask;
    uint8_t shift;
    uint8_t count;
};

enum Flags {
    Wide = (1 << 0),
    IPInc = (1 << 1),
    CSInc = (1 << 2),
    RmIsWide = (1 << 3)
};

struct Entry {
    Operation mnemonic;
    Bits bits[Field::Field_count];
    uint16_t flags;
};

enum OperandType {
    OpType_none, 

    OpType_register,
    OpType_effectiveAddrCalc,
    OpType_immediate,
    OpType_jmp,

    OpType_count
};

struct Jump {
    uint16_t ipAddress;
    uint16_t csAddress;
};

struct Operand {
    OperandType type;
    union {
        RegisterAccess reg;
        EffectiveAddrExpression expression;
        int16_t immediate;
        uint32_t address;
        Jump jmp;
    };
};

struct Instruction {
    Operation op;
    uint32_t address;
    uint16_t size;
    uint16_t flags;
    Operand operands[2];
};

// NOTE - These are memory operations, could be moved to its own Memory.h file. Leaving it here for now
uint8_t ReadByteFromMemory(SegmentedAddress at);
uint16_t ReadWordFromMemory(SegmentedAddress at);

void WriteByteToMemory(uint16_t value, SegmentedAddress at);
void WriteWordToMemory(uint16_t value, SegmentedAddress at);

uint8_t FetchNextInstructionByte(CPU &cpu);
SegmentedAddress Create(uint16_t segment, uint16_t offset);