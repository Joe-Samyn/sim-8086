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
    Register_c,
    Register_d,
    Register_b,
    Register_sp,
    Register_bp,
    Register_si,
    Register_di,

    Register_none,

    Register_count
};

enum SegmentRegisters {
    CS,
    SS,
    DS,
    ES,

    Segment_count
};

enum CPUFlags {
    Overflow = (1 << 0),
    Direction = (1 << 1),
    Interrupt = (1 << 2),
    Trap = (1 << 3),
    Sign = (1 << 4),
    Zero = (1 << 5),
    AuxCarry = (1 << 6),
    Parity = (1 << 7),
    Carry = (1 << 8)
};

struct CPU {
    uint16_t IP;
    uint16_t flags;
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
    uint8_t code;      // index of the register in the 8086 manual. For example, register AX/AL is 000 while register CX/CL is 001
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

/**
 * Types of effective address calculations supported on the Intel 8086 processor.
 */
enum EAType: uint8_t {
    Direct_address,

    Bx_si,
    Bx_di,
    Bp_si,
    Bp_di,
    Si,
    Di,
    Bp,
    Bx,

    EAType_count
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

// TODO: Determine where to host constant variables like this.
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
    int16_t displacement;

    union {
        RegisterAccess reg;
        EAType ea;
        int16_t immediate;
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
void ComputeOF(CPU &cpu, int16_t src, int16_t dest, int16_t result, uint8_t size);
void ComputeSF(CPU &cpu, int16_t result, uint8_t size);
void ComputeZF(CPU &cpu, uint16_t result, uint8_t size);
void ComputeCF(CPU &cpu, uint16_t src, uint16_t dest, uint16_t result, uint8_t size, bool invert = false);
