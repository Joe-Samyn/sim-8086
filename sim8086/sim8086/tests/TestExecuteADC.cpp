
#include "Assert.h"
#include "TestFixture.h"

#include "Sim8086.h"
#include "Execute.h"

/*
    NOTE: The ADC functionality reuses the ExecuteAdd function. A simple boolean is set in the function
    parameter to tell the arithmetic function to use the carry. Therefore, these tests only validate
    the carry is included in the arithmetic. All other addition test cases are in `TestExecuteADD.cpp`. 
*/

TEST(ExecuteAdd_FullRegisterToFullRegisterWithCarryCfIsOne, {
    // ARRANGE
    CPU cpu = {};
    cpu.flags |= Carry;
    cpu.registers[Register_b] = 0x1122;
    cpu.registers[Register_c] = 0x1234;

    uint8_t size = WIDE;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_b;
    src.reg.offset = FULL_BITS;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_c;
    dest.reg.offset = FULL_BITS;

    int exp = 0x2357;

    // ACT
    ExecuteAdd(cpu, src, dest, size, true);
    uint16_t result = cpu.registers[Register_c];

    // ASSERT
    ASSERT_EQUAL(exp, result);
})

TEST(ExecuteAdd_FullRegisterToFullRegisterWithCarryCfIsZero, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0x1122;
    cpu.registers[Register_c] = 0x1234;

    uint8_t size = WIDE;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_b;
    src.reg.offset = FULL_BITS;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_c;
    dest.reg.offset = FULL_BITS;

    int exp = 0x2356;

    // ACT
    ExecuteAdd(cpu, src, dest, size, true);
    uint16_t result = cpu.registers[Register_c];

    // ASSERT
    ASSERT_EQUAL(exp, result);
})

TEST(ExecuteAdd_LoByteToLoByteWithCarryCfIsOne, {
    // ARRANGE
    CPU cpu = {};
    cpu.flags = Carry;
    cpu.registers[Register_b] = 0xAB11;
    cpu.registers[Register_c] = 0xCD24;

    uint8_t size = BYTE;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_b;
    src.reg.offset = LO_BITS;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_c;
    dest.reg.offset = LO_BITS;

    int exp = 0xCD36;
    int expLo = 0x36;

    // ACT
    ExecuteAdd(cpu, src, dest, size, true);
    uint16_t result = cpu.registers[Register_c];
    uint8_t resultLo = ReadLoByte(cpu.registers[Register_c]);

    // ASSERT
    ASSERT_EQUAL(exp, result);
    ASSERT_EQUAL(expLo, resultLo);
})

TESTS(ExecuteAdcTests) = {
    ExecuteAdd_FullRegisterToFullRegisterWithCarryCfIsOne,
    ExecuteAdd_FullRegisterToFullRegisterWithCarryCfIsZero,
    ExecuteAdd_LoByteToLoByteWithCarryCfIsOne
};