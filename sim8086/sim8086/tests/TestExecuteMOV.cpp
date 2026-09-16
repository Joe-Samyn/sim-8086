
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

TEST(ExecuteMov_FullRegisterToFullRegisterMov, {
    // Arrange
    CPU cpu = {};
    cpu.registers[Register_b] = 0x1234;

    uint8_t size = WIDE;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_b;
    src.reg.offset = FULL_BITS;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_a;
    dest.reg.offset = FULL_BITS;

    int exp = 0x1234;

    // Act
    ExecuteMov(cpu, src, dest, size);
    uint16_t result = cpu.registers[Register_a];

    // Assert
    ASSERT_EQUAL(exp, result);
})

TEST(ExecuteMov_LoByteToLoByteRegisterMov, {
    // Arrange
    CPU cpu = {};
    cpu.registers[Register_b] = 0x99AB;
    cpu.registers[Register_a] = 0xCD00;

    uint8_t size = BYTE;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_b;
    src.reg.offset = LO_BITS;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_a;
    dest.reg.offset = LO_BITS;

    int expLo = 0xAB;
    int expHi = 0xCD;

    // Act
    ExecuteMov(cpu, src, dest, size);
    uint8_t resultLo = ReadLoByte(cpu.registers[Register_a]);
    uint8_t resultHi = ReadHiByte(cpu.registers[Register_a]);

    // Assert
    ASSERT_EQUAL(expLo, resultLo);
    ASSERT_EQUAL(expHi, resultHi);
})

TEST(ExecuteMov_HiByteToHiByteMov, {
    // Arrange
    CPU cpu = {};
    cpu.registers[Register_b] = 0x9900;
    cpu.registers[Register_a] = 0x00CD;

    uint8_t size = BYTE;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_b;
    src.reg.offset = HI_BITS;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_a;
    dest.reg.offset = HI_BITS;

    int expHi = 0x99;
    int expLo = 0xCD;

    // Act
    ExecuteMov(cpu, src, dest, size);
    uint8_t resultHi = ReadHiByte(cpu.registers[Register_a]);
    uint8_t resultLo = ReadLoByte(cpu.registers[Register_a]);

    // Assert
    ASSERT_EQUAL(expHi, resultHi);
    ASSERT_EQUAL(expLo, resultLo);
})

TEST(ExecuteMov_HiByteToLoByte, {
    // Arrange
    CPU cpu = {};
    cpu.registers[Register_b] = 0x7799;
    cpu.registers[Register_a] = 0xAA55;

    uint8_t size = BYTE;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_b;
    src.reg.offset = HI_BITS;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_a;
    dest.reg.offset = LO_BITS;

    int expLo = 0x77;
    int expHi = 0xAA;

    // Act
    ExecuteMov(cpu, src, dest, size);
    uint8_t resultLo = ReadLoByte(cpu.registers[Register_a]);
    uint8_t resultHi = ReadHiByte(cpu.registers[Register_a]);

    // Assert
    ASSERT_EQUAL(expLo, resultLo);
    ASSERT_EQUAL(expHi, resultHi);
})

TEST(ExecuteMov_LoByteToHiByte, {
    // Arrange
    CPU cpu = {};
    cpu.registers[Register_b] = 0x9933;
    cpu.registers[Register_a] = 0x1144;

    uint8_t size = BYTE;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_b;
    src.reg.offset = LO_BITS;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_a;
    dest.reg.offset = HI_BITS;

    int expHi = 0x33;
    int expLo = 0x44;

    // Act
    ExecuteMov(cpu, src, dest, size);
    uint8_t resultHi = ReadHiByte(cpu.registers[Register_a]);
    uint8_t resultLo = ReadLoByte(cpu.registers[Register_a]);

    // Assert
    ASSERT_EQUAL(expHi, resultHi);
    ASSERT_EQUAL(expLo, resultLo);
})

TEST(ExecuteMov_DirectAddressToFullRegister, {
    // Arrange
    CPU cpu = {};
    SegmentedAddress at = Create(cpu.segmentRegisters[DS], 0x0100);
    WriteWordToMemory(0xBEEF, at);

    uint8_t size = WIDE;

    Operand src = {};
    src.type = OpType_effectiveAddrCalc;
    src.ea = Direct_address;
    src.displacement = 0x0100;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_a;
    dest.reg.offset = FULL_BITS;

    int exp = 0xBEEF;

    // Act
    ExecuteMov(cpu, src, dest, size);
    uint16_t result = cpu.registers[Register_a];

    // Assert
    ASSERT_EQUAL(exp, result);
})

TEST(ExecuteMov_DirectAddressToLoByte, {
    // Arrange
    CPU cpu = {};
    cpu.registers[Register_a] = 0xFF00;
    SegmentedAddress at = Create(cpu.segmentRegisters[DS], 0x0200);
    WriteByteToMemory(0x7A, at);

    uint8_t size = BYTE;

    Operand src = {};
    src.type = OpType_effectiveAddrCalc;
    src.ea = Direct_address;
    src.displacement = 0x0200;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_a;
    dest.reg.offset = LO_BITS;

    int expLo = 0x7A;
    int expHi = 0xFF;

    // Act
    ExecuteMov(cpu, src, dest, size);
    uint8_t resultLo = ReadLoByte(cpu.registers[Register_a]);
    uint8_t resultHi = ReadHiByte(cpu.registers[Register_a]);

    // Assert
    ASSERT_EQUAL(expLo, resultLo);
    ASSERT_EQUAL(expHi, resultHi);
})

TEST(ExecuteMov_DirectAddressToHiByte, {
    // Arrange
    CPU cpu = {};
    cpu.registers[Register_a] = 0x00DD;
    SegmentedAddress at = Create(cpu.segmentRegisters[DS], 0x0300);
    WriteByteToMemory(0x3C, at);

    uint8_t size = BYTE;

    Operand src = {};
    src.type = OpType_effectiveAddrCalc;
    src.ea = Direct_address;
    src.displacement = 0x0300;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_a;
    dest.reg.offset = HI_BITS;

    int expHi = 0x3C;
    int expLo = 0xDD;

    // Act
    ExecuteMov(cpu, src, dest, size);
    uint8_t resultHi = ReadHiByte(cpu.registers[Register_a]);
    uint8_t resultLo = ReadLoByte(cpu.registers[Register_a]);

    // Assert
    ASSERT_EQUAL(expHi, resultHi);
    ASSERT_EQUAL(expLo, resultLo);
})

TEST(ExecuteMov_EffectiveAddressToFullRegister, {
    // Arrange
    CPU cpu = {};
    cpu.registers[Register_b] = 0x2000;
    cpu.registers[Register_si] = 0x0005;

    SegmentedAddress at = Create(cpu.segmentRegisters[DS], 0x2015);
    WriteWordToMemory(0xABCD, at);

    uint8_t size = WIDE;

    Operand src = {};
    src.type = OpType_effectiveAddrCalc;
    src.ea = Bx_si;
    src.displacement = 0x10;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_a;
    dest.reg.offset = FULL_BITS;

    int exp = 0xABCD;

    // Act
    ExecuteMov(cpu, src, dest, size);
    uint16_t result = cpu.registers[Register_a];

    // Assert
    ASSERT_EQUAL(exp, result);
})

TEST(ExecuteMov_EffectiveAddressToLowByte, {
    // Arrange
    CPU cpu = {};
    cpu.registers[Register_a] = 0xFF00;
    cpu.registers[Register_b] = 0x2000;
    cpu.registers[Register_si] = 0x0005;

    SegmentedAddress at = Create(cpu.segmentRegisters[DS], 0x2015);
    WriteByteToMemory(0x5E, at);

    uint8_t size = BYTE;

    Operand src = {};
    src.type = OpType_effectiveAddrCalc;
    src.ea = Bx_si;
    src.displacement = 0x10;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_a;
    dest.reg.offset = LO_BITS;

    int expLo = 0x5E;
    int expHi = 0xFF;

    // Act
    ExecuteMov(cpu, src, dest, size);
    uint8_t resultLo = ReadLoByte(cpu.registers[Register_a]);
    uint8_t resultHi = ReadHiByte(cpu.registers[Register_a]);

    // Assert
    ASSERT_EQUAL(expLo, resultLo);
    ASSERT_EQUAL(expHi, resultHi);
})

TEST(ExecuteMov_EffectiveAddressToHiByte, {
    // Arrange
    CPU cpu = {};
    cpu.registers[Register_a] = 0x00DD;
    cpu.registers[Register_b] = 0x2000;
    cpu.registers[Register_si] = 0x0005;

    SegmentedAddress at = Create(cpu.segmentRegisters[DS], 0x2015);
    WriteByteToMemory(0x88, at);

    uint8_t size = BYTE;

    Operand src = {};
    src.type = OpType_effectiveAddrCalc;
    src.ea = Bx_si;
    src.displacement = 0x10;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_a;
    dest.reg.offset = HI_BITS;

    int expHi = 0x88;
    int expLo = 0xDD;

    // Act
    ExecuteMov(cpu, src, dest, size);
    uint8_t resultHi = ReadHiByte(cpu.registers[Register_a]);
    uint8_t resultLo = ReadLoByte(cpu.registers[Register_a]);

    // Assert
    ASSERT_EQUAL(expHi, resultHi);
    ASSERT_EQUAL(expLo, resultLo);
})

TEST(ExecuteMov_FullRegisterToEffectiveAddress, {
    // Arrange
    CPU cpu = {};
    cpu.registers[Register_a] = 0x4321;
    cpu.registers[Register_b] = 0x3000;
    cpu.registers[Register_di] = 0x0002;

    uint8_t size = WIDE;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_a;
    src.reg.offset = FULL_BITS;

    Operand dest = {};
    dest.type = OpType_effectiveAddrCalc;
    dest.ea = Bx_di;
    dest.displacement = 0;

    int exp = 0x4321;

    // Act
    ExecuteMov(cpu, src, dest, size);
    SegmentedAddress at = Create(cpu.segmentRegisters[DS], 0x3002);
    uint16_t result = ReadWordFromMemory(at);

    // Assert
    ASSERT_EQUAL(exp, result);
})

TEST(ExecuteMov_LoByteOfRegisterToEffectiveAddress, {
    // Arrange
    CPU cpu = {};
    cpu.registers[Register_a] = 0x9977;
    cpu.registers[Register_b] = 0x3000;
    cpu.registers[Register_di] = 0x0002;

    uint8_t size = BYTE;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_a;
    src.reg.offset = LO_BITS;

    Operand dest = {};
    dest.type = OpType_effectiveAddrCalc;
    dest.ea = Bx_di;
    dest.displacement = 0;

    int exp = 0x77;

    // Act
    ExecuteMov(cpu, src, dest, size);
    SegmentedAddress at = Create(cpu.segmentRegisters[DS], 0x3002);
    uint8_t result = ReadByteFromMemory(at);

    // Assert
    ASSERT_EQUAL(exp, result);
})

TEST(ExecuteMov_HiByteOfRegisterToEffectiveAddress, {
    // Arrange
    CPU cpu = {};
    cpu.registers[Register_a] = 0x8825;
    cpu.registers[Register_b] = 0x3000;
    cpu.registers[Register_di] = 0x0002;

    uint8_t size = BYTE;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_a;
    src.reg.offset = HI_BITS;

    Operand dest = {};
    dest.type = OpType_effectiveAddrCalc;
    dest.ea = Bx_di;
    dest.displacement = 0;

    int exp = 0x88;

    // Act
    ExecuteMov(cpu, src, dest, size);
    SegmentedAddress at = Create(cpu.segmentRegisters[DS], 0x3002);
    uint8_t result = ReadByteFromMemory(at);

    // Assert
    ASSERT_EQUAL(exp, result);
})


TESTS(ExecuteMovTests) = {
    ExecuteMov_ExecutesImmediateToFullRegister,
    ExecuteMov_ExecutesImmediateToLowBytesOfRegister,
    ExecuteMov_ExecutesImmediateToHiBytesOfRegister,
    ExecuteMov_FullRegisterToFullRegisterMov,
    ExecuteMov_LoByteToLoByteRegisterMov,
    ExecuteMov_HiByteToHiByteMov,
    ExecuteMov_HiByteToLoByte,
    ExecuteMov_LoByteToHiByte,
    ExecuteMov_DirectAddressToFullRegister,
    ExecuteMov_DirectAddressToLoByte,
    ExecuteMov_DirectAddressToHiByte,
    ExecuteMov_EffectiveAddressToFullRegister,
    ExecuteMov_EffectiveAddressToLowByte,
    ExecuteMov_EffectiveAddressToHiByte,
    ExecuteMov_FullRegisterToEffectiveAddress,
    ExecuteMov_LoByteOfRegisterToEffectiveAddress,
    ExecuteMov_HiByteOfRegisterToEffectiveAddress
};