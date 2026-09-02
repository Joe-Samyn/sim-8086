#include "Assert.h"
#include "TestFixture.h"

#include "Sim8086.h"
#include "Execute.h"

TEST(ExecuteSBB_FullRegFromFullRegCarryIsZero, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0x2468;
    cpu.registers[Register_c] = 0x1357;

    uint8_t size = WIDE;
    bool useCarry = true;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_b;
    dest.reg.offset = FULL_BITS;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_c;
    src.reg.offset = FULL_BITS;

    uint16_t exp = 0x1111;

    // ACT
    ExecuteSub(cpu, src, dest, size, useCarry);
    uint16_t result = cpu.registers[Register_b];

    // ASSERT
    ASSERT_EQUAL(result, exp);
})

TEST(ExecuteSub_FullRegFromFullRegCarryIsOne, {
    // ARRANGE
    CPU cpu = {};
    cpu.flags |= Carry;
    cpu.registers[Register_b] = 0x8635;
    cpu.registers[Register_c] = 0x2213;

    uint8_t size = WIDE;
    bool useCarry = true;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_b;
    dest.reg.offset = FULL_BITS;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_c;
    src.reg.offset = FULL_BITS;

    uint16_t exp = 0x6421;

    // ACT
    ExecuteSub(cpu, src, dest, size, useCarry);
    uint16_t result = cpu.registers[Register_b];

    // ASSERT
    ASSERT_EQUAL(result, exp);
})

TEST(ExecuteSub_LoByteFromLoByteCarryIsOne, {
    // ARRANGE
    CPU cpu = {};
    cpu.flags |= Carry;
    cpu.registers[Register_b] = 0x2468;
    cpu.registers[Register_c] = 0x1357;

    uint8_t size = BYTE;
    bool useCarry = true;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_b;
    dest.reg.offset = LO_BITS;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_c;
    src.reg.offset = LO_BITS;

    uint16_t exp = 0x2410;
    uint8_t expLo = 0x10;

    // ACT
    ExecuteSub(cpu, src, dest, size, useCarry);
    uint16_t result = cpu.registers[Register_b];
    uint8_t resultLo = ReadLoByte(cpu.registers[Register_b]);

    // ASSERT
    ASSERT_EQUAL(result, exp);
    ASSERT_EQUAL(resultLo, expLo);
})

TESTS(ExecuteSbbTests) = {
    ExecuteSBB_FullRegFromFullRegCarryIsZero,
    ExecuteSub_FullRegFromFullRegCarryIsOne,
    ExecuteSub_LoByteFromLoByteCarryIsOne
};