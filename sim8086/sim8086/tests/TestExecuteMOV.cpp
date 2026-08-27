
#include "TestFixture.h"
#include "Assert.h"
#include "Execute.h"

TEST(ExecuteMov_ExecutesImmediateToFullRegister, {
    // Arrange 
    CPU cpu = {};

    uint8_t size = WIDE;

    Operand src;
    src.type = OpType_immediate;
    src.immediate = 120;

    Operand dest;
    dest.type = OpType_register;
    dest.reg.index = Register_a;
    dest.reg.offset = FULL_BITS;

    int exp = 120;

    // Act
    ExecuteMov(cpu, src, dest, size);
    uint16_t result = cpu.registers[Register_a];

    // Assert

    ASSERT_EQUAL(exp, result);

})

TEST(ExecuteMov_ExecutesImmediateToLowBytesOfRegister, {
    // Arrange 
    CPU cpu = {};

    uint8_t size = BYTE;

    Operand src;
    src.type = OpType_immediate;
    src.immediate = 25;

    Operand dest;
    dest.type = OpType_register;
    dest.reg.index = Register_a;
    dest.reg.offset = LO_BITS;

    int exp = 25;

    // Act
    ExecuteMov(cpu, src, dest, size);
    uint8_t result = ReadLoByte(cpu.registers[Register_a]);

    // Assert

    ASSERT_EQUAL(exp, result);
})

TEST(ExecuteMov_ExecutesImmediateToHiBytesOfRegister, {
    // Arrange 
    CPU cpu = {};

    uint8_t size = BYTE;

    Operand src;
    src.type = OpType_immediate;
    src.immediate = 25;

    Operand dest;
    dest.type = OpType_register;
    dest.reg.index = Register_a;
    dest.reg.offset =HI_BITS;

    int exp = 25;

    // Act
    ExecuteMov(cpu, src, dest, size);
    uint8_t result = ReadHiByte(cpu.registers[Register_a]);

    // Assert
    ASSERT_EQUAL(exp, result);
})


TESTS(ExecuteMovTests) = {
    ExecuteMov_ExecutesImmediateToFullRegister,
    ExecuteMov_ExecutesImmediateToLowBytesOfRegister,
    ExecuteMov_ExecutesImmediateToHiBytesOfRegister
};