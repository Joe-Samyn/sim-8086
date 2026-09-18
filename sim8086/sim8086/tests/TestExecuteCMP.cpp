#include "Assert.h"
#include "TestFixture.h"

#include "Sim8086.h"
#include "Execute.h"

TEST(ExecuteCmp_NoFlagsSet, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0x4567;
    cpu.registers[Register_c] = 0x1122;

    uint8_t size = WIDE;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.code = Register_b;
    dest.reg.offset = FULL_BITS;

    Operand src = {};
    src.type = OpType_register;
    src.reg.code = Register_c;
    src.reg.offset = FULL_BITS;

    uint16_t exp = 0;
    uint16_t expRegB = 0x4567;
    uint16_t expRegC = 0x1122;

    // ACT
    ExecuteCmp(cpu, src, dest, size);
    uint16_t result = cpu.flags;
    uint16_t resultRegB = cpu.registers[Register_b];
    uint16_t resultRegC = cpu.registers[Register_c];

    // ASSERT
    ASSERT_EQUAL(result, exp);
    ASSERT_EQUAL(resultRegB, expRegB);
    ASSERT_EQUAL(resultRegC, expRegC);
})

TEST(ExecuteCmp_SetCFFlag, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0x1122;
    cpu.registers[Register_c] = 0x4567;

    uint8_t size = WIDE;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.code = Register_b;
    dest.reg.offset = FULL_BITS;

    Operand src = {};
    src.type = OpType_register;
    src.reg.code = Register_c;
    src.reg.offset = FULL_BITS;

    uint16_t exp = Carry | Sign;
    uint16_t expRegB = 0x1122;
    uint16_t expRegC = 0x4567;

    // ACT
    ExecuteCmp(cpu, src, dest, size);
    uint16_t result = cpu.flags;
    uint16_t resultRegB = cpu.registers[Register_b];
    uint16_t resultRegC = cpu.registers[Register_c];

    // ASSERT
    ASSERT_EQUAL(result, exp);
    ASSERT_EQUAL(resultRegB, expRegB);
    ASSERT_EQUAL(resultRegC, expRegC);
})

TEST(ExecuteCmp_SetZF, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0x1122;
    cpu.registers[Register_c] = 0x1122;

    uint8_t size = WIDE;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.code = Register_b;
    dest.reg.offset = FULL_BITS;

    Operand src = {};
    src.type = OpType_register;
    src.reg.code = Register_c;
    src.reg.offset = FULL_BITS;

    uint16_t exp = Zero;
    uint16_t expRegB = 0x1122;
    uint16_t expRegC = 0x1122;

    // ACT
    ExecuteCmp(cpu, src, dest, size);
    uint16_t result = cpu.flags;
    uint16_t resultRegB = cpu.registers[Register_b];
    uint16_t resultRegC = cpu.registers[Register_c];

    // ASSERT
    ASSERT_EQUAL(result, exp);
    ASSERT_EQUAL(resultRegB, expRegB);
    ASSERT_EQUAL(resultRegC, expRegC);
})

TEST(ExecuteCmp_SetOF, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0x8001;
    cpu.registers[Register_c] = 0x43AB;

    uint8_t size = WIDE;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.code = Register_b;
    dest.reg.offset = FULL_BITS;

    Operand src = {};
    src.type = OpType_register;
    src.reg.code = Register_c;
    src.reg.offset = FULL_BITS;

    uint16_t exp = Overflow;
    uint16_t expRegB = 0x8001;
    uint16_t expRegC = 0x43AB;

    // ACT
    ExecuteCmp(cpu, src, dest, size);
    uint16_t result = cpu.flags;
    uint16_t resultRegB = cpu.registers[Register_b];
    uint16_t resultRegC = cpu.registers[Register_c];

    // ASSERT
    ASSERT_EQUAL(result, exp);
    ASSERT_EQUAL(resultRegB, expRegB);
    ASSERT_EQUAL(resultRegC, expRegC);
})

TEST(ExecuteCmp_SetSF, {

    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0xFF04;
    cpu.registers[Register_c] = 0x0003;

    uint8_t size = WIDE;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.code = Register_b;
    dest.reg.offset = FULL_BITS;

    Operand src = {};
    src.type = OpType_register;
    src.reg.code = Register_c;
    src.reg.offset = FULL_BITS;

    uint16_t exp = Sign;
    uint16_t expRegB = 0xFF04;
    uint16_t expRegC = 0x0003;

    // ACT
    ExecuteCmp(cpu, src, dest, size);
    uint16_t result = cpu.flags;
    uint16_t resultRegB = cpu.registers[Register_b];
    uint16_t resultRegC = cpu.registers[Register_c];

    // ASSERT
    ASSERT_EQUAL(result, exp);
    ASSERT_EQUAL(resultRegB, expRegB);
    ASSERT_EQUAL(resultRegC, expRegC);
})

// TODO: Add BYTE size test cases 


TESTS(ExecuteCmpTests) = {
    ExecuteCmp_NoFlagsSet,
    ExecuteCmp_SetCFFlag,
    ExecuteCmp_SetZF,
    ExecuteCmp_SetOF,
    ExecuteCmp_SetSF
};
