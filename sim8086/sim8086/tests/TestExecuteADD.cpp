
#include "Assert.h"
#include "TestFixture.h"

#include "Sim8086.h"
#include "Execute.h"

TEST(ExecuteAdd_FullRegisterToFullRegisterNoFlagsSet, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_a] = 0xABC;
    cpu.registers[Register_b] = 0xDEF;

    uint8_t size = WIDE;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_a;
    src.reg.offset = FULL_BITS;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_b;
    dest.reg.offset = FULL_BITS;

    int exp = 0x18AB;

    // ACT
    ExecuteAdd(cpu, src, dest, size);
    uint16_t result = cpu.registers[Register_b];

    // ASSERT
    ASSERT_EQUAL(exp, result);
})

TEST(ExecuteAdd_FullRegisterToFullRegisterCarryFlagSet, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0xFEFE;
    cpu.registers[Register_a] = 0x1A1A;

    uint8_t size = WIDE;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_b;
    src.reg.offset = FULL_BITS;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_a;
    dest.reg.offset = FULL_BITS;

    int exp = 0x1918;

    // ACT
    ExecuteAdd(cpu, src, dest, size);
    uint16_t result = cpu.registers[Register_a];
    uint16_t cf = cpu.flags & Carry;
    uint16_t of = cpu.flags & Overflow;

    // ASSERT
    ASSERT_EQUAL(exp, result);
    ASSERT_EQUAL(Carry, cf);
    ASSERT_EQUAL(0, of);
})

TEST(ExecuteAdd_FullRegisterToFullRegisterOverflowFlagSet, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0x7A00;
    cpu.registers[Register_c] = 0x0ABC;

    uint8_t size = WIDE;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_c;
    src.reg.offset = FULL_BITS;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_b;
    dest.reg.offset = FULL_BITS;

    int exp = 0x84BC;

    // ACT
    ExecuteAdd(cpu, src, dest, size);
    uint16_t result = cpu.registers[Register_b];
    uint16_t of = cpu.flags & Overflow;
    uint16_t cf = cpu.flags & Carry;

    // ASSERT
    ASSERT_EQUAL(exp, result);
    ASSERT_EQUAL(Overflow, of);
    ASSERT_EQUAL(0, cf);
})

TEST(ExecuteAdd_FullRegisterToFullRegisterZeroFlagSet, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0x5;
    cpu.registers[Register_c] = 0xFFFB;

    uint8_t size = WIDE;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_c;
    src.reg.offset = FULL_BITS;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_b;
    dest.reg.offset = FULL_BITS;

    int exp = 0;

    // ACT
    ExecuteAdd(cpu, src, dest, size);
    uint16_t result = cpu.registers[Register_b];
    uint16_t zf = cpu.flags & Zero;

    // ASSERT
    ASSERT_EQUAL(exp, result);
    ASSERT_EQUAL(Zero, zf);
})

TEST(ExecuteAdd_FullRegisterToFullRegisterSignFlagSet, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0xF000;
    cpu.registers[Register_c] = 0x5;

    uint8_t size = WIDE;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_c;
    src.reg.offset = FULL_BITS;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_b;
    dest.reg.offset = FULL_BITS;

    int exp = 0xF005;

    // ACT
    ExecuteAdd(cpu, src, dest, size);
    uint16_t result = cpu.registers[Register_b];
    uint16_t sf = cpu.flags & Sign;

    // ASSERT
    ASSERT_EQUAL(exp, result);
    ASSERT_EQUAL(Sign, sf);
})

TEST(ExecuteAdd_LoRegisterToLoRegisterAddNoFlagsSet, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0xFF01;
    cpu.registers[Register_c] = 0xFF02;

    uint8_t size = BYTE;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_b;
    src.reg.offset = LO_BITS;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_c;
    dest.reg.offset = LO_BITS;

    int exp = 0x03;

    // ACT
    ExecuteAdd(cpu, src, dest, size);
    uint8_t result = ReadLoByte(cpu.registers[Register_c]);

    // ASSERT
    ASSERT_EQUAL(exp, result);
    ASSERT_EQUAL(0, cpu.flags);
})

TEST(ExecuteAdd_LoByteToLoByteCarryFlagSet, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0x00FF;
    cpu.registers[Register_c] = 0xAB02;

    uint8_t size = BYTE;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_b;
    src.reg.offset = LO_BITS;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_c;
    dest.reg.offset = LO_BITS;

    int exp = 0xAB01;
    int expLo = 0x01;

    // ACT
    ExecuteAdd(cpu, src, dest, size);
    uint16_t result = cpu.registers[Register_c];
    uint8_t resultLo = ReadLoByte(cpu.registers[Register_c]);
    uint16_t cf = cpu.flags & Carry;

    // ASSERT
    ASSERT_EQUAL(exp, result);
    ASSERT_EQUAL(expLo, resultLo);
    ASSERT_EQUAL(Carry, cf);
})

TEST(ExecuteAdd_LoByteToLoByteSignFlagSet, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0xFFF1;
    cpu.registers[Register_c] = 0xAB01;

    uint8_t size = BYTE;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_b;
    src.reg.offset = LO_BITS;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_c;
    dest.reg.offset = LO_BITS;

    int exp = 0xABF2;
    int expLo = 0xF2;

    // ACT
    ExecuteAdd(cpu, src, dest, size);
    uint16_t result = cpu.registers[Register_c];
    uint8_t resultLo = ReadLoByte(cpu.registers[Register_c]);
    uint16_t sf = cpu.flags & Sign;

    // ASSERT
    ASSERT_EQUAL(exp, result);
    ASSERT_EQUAL(expLo, resultLo);
    ASSERT_EQUAL(Sign, sf);
})

TEST(ExecuteAdd_LoByteToLoByteOverflowFlagSet, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0x1155;
    cpu.registers[Register_c] = 0x1156;

    uint8_t size = BYTE;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_b;
    src.reg.offset = LO_BITS;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_c;
    dest.reg.offset = LO_BITS;

    int exp = 0x11AB;
    int expLo = 0xAB;

    // ACT
    ExecuteAdd(cpu, src, dest, size);
    uint16_t result = cpu.registers[Register_c];
    uint8_t resultLo = ReadLoByte(cpu.registers[Register_c]);
    uint16_t of = cpu.flags & Overflow;

    // ASSERT
    ASSERT_EQUAL(exp, result);
    ASSERT_EQUAL(expLo, resultLo);
    ASSERT_EQUAL(Overflow, of);
})

TEST(ExecuteAdd_LoByteToLoByteZeroFlagSet, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0x11FF;
    cpu.registers[Register_c] = 0x1101;

    uint8_t size = BYTE;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_b;
    src.reg.offset = LO_BITS;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_c;
    dest.reg.offset = LO_BITS;

    int exp = 0x1100;
    int expLo = 0x00;

    // ACT
    ExecuteAdd(cpu, src, dest, size);
    uint16_t result = cpu.registers[Register_c];
    uint8_t resultLo = ReadLoByte(cpu.registers[Register_c]);
    uint16_t zf = cpu.flags & Zero;

    // ASSERT
    ASSERT_EQUAL(exp, result);
    ASSERT_EQUAL(expLo, resultLo);
    ASSERT_EQUAL(Zero, zf);
})

TEST(ExecuteAdd_LoByteToHiByte, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0x01AB;
    cpu.registers[Register_c] = 0xCD20;

    uint8_t size = BYTE;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_c;
    src.reg.offset = LO_BITS;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_b;
    dest.reg.offset = HI_BITS;

    int exp = 0x21AB;
    int expHi = 0x21;

    // ACT
    ExecuteAdd(cpu, src, dest, size);
    uint16_t result = cpu.registers[Register_b];
    uint8_t resultHi = ReadHiByte(cpu.registers[Register_b]);

    // ASSERT
    ASSERT_EQUAL(exp, result);
    ASSERT_EQUAL(expHi, resultHi);
})

TEST(ExecuteAdd_HiByteToLoByte, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0x01AB;
    cpu.registers[Register_c] = 0xCD22;

    uint8_t size = BYTE;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_b;
    src.reg.offset = HI_BITS;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_c;
    dest.reg.offset = LO_BITS;

    int exp = 0xCD23;
    int expLo = 0x23;

    // ACT
    ExecuteAdd(cpu, src, dest, size);
    uint16_t result = cpu.registers[Register_c];
    uint8_t resultLo = ReadLoByte(cpu.registers[Register_c]);

    // ASSERT
    ASSERT_EQUAL(exp, result);
    ASSERT_EQUAL(expLo, resultLo);
})

TESTS(ExecuteAddTests) = {
    ExecuteAdd_FullRegisterToFullRegisterNoFlagsSet,
    ExecuteAdd_FullRegisterToFullRegisterCarryFlagSet,
    ExecuteAdd_FullRegisterToFullRegisterOverflowFlagSet,
    ExecuteAdd_FullRegisterToFullRegisterZeroFlagSet,
    ExecuteAdd_FullRegisterToFullRegisterSignFlagSet,
    ExecuteAdd_LoRegisterToLoRegisterAddNoFlagsSet,
    ExecuteAdd_LoByteToLoByteCarryFlagSet,
    ExecuteAdd_LoByteToLoByteSignFlagSet,
    ExecuteAdd_LoByteToLoByteOverflowFlagSet,
    ExecuteAdd_LoByteToLoByteZeroFlagSet,
    ExecuteAdd_LoByteToHiByte,
    ExecuteAdd_HiByteToLoByte
};