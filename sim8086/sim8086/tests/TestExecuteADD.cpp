
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

TESTS(ExecuteAddTests) = {
    ExecuteAdd_FullRegisterToFullRegisterNoFlagsSet,
    ExecuteAdd_FullRegisterToFullRegisterCarryFlagSet,
    ExecuteAdd_FullRegisterToFullRegisterOverflowFlagSet,
    ExecuteAdd_FullRegisterToFullRegisterZeroFlagSet,
    ExecuteAdd_FullRegisterToFullRegisterSignFlagSet
};