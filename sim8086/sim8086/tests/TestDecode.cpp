//
// Created by Joe Samyn on 8/21/26.
//

#include "Assert.h"
#include "TestFixture.h"
#include "Sim8086.h"
#include "Decode.h"


TEST(DecodeRegister_DecodesWideRegisterSuccessfully, {
    // Arrange
    uint8_t reg = 0b00;
    uint8_t size = WIDE;
    RegisterAccess exp;
    exp.index = Register_a;
    exp.offset = FULL_BITS;

    RegisterAccess res;

    // Act
    DecodeRegister(reg, size, res);

    // Assert
    ASSERT_EQUAL(res.index, exp.index);
    ASSERT_EQUAL(res.offset, exp.offset);
})

TEST(DecodeRegister_DecodesByteRegisterSuccessfully, {
    // Arrange
    uint8_t reg = 0b00;
    uint8_t size = BYTE;
    RegisterAccess exp;
    exp.index = Register_a;
    exp.offset = LO_BITS;

    RegisterAccess res;

    // Act
    DecodeRegister(reg, size, res);

    // Assert
    ASSERT_EQUAL(res.index, exp.index);
    ASSERT_EQUAL(res.offset, exp.offset);
})

// TEST(DecodeRegister_RaisesErrorOnInvalidRegister, {
//     // TODO
// })

// TEST(DecodeRegister_RaisesErrorOnInvalidSize, {
//     // TODO
// })

TEST(DecodeEffectiveAddrExpression_SuccessfullyDecodesRmModIntoEffectiveAddrExpression, {
    // Arrange
    uint8_t mod = Memory_mode_8_bit_disp;
    uint8_t rm = 0b110;
    SegmentedAddress at = Create(0, 0);
    EffectiveAddrExpression exp = {};
    exp.calculationType = Effective_addr_bp;
    exp.base.index = Register_bp;
    exp.base.offset = FULL_BITS;

    EffectiveAddrExpression res = {};

    // Act
    DecodeEffectiveAddrExpression(mod, rm, res, at);

    // Assert
    ASSERT_EQUAL(res.calculationType, exp.calculationType);
    ASSERT_EQUAL(res.base.index, exp.base.index);
    ASSERT_EQUAL(res.base.offset, exp.base.offset);
})

TEST(DecodeEffectiveAddrExpression_SuccessfullyDecodesDirectAddressIntoEffectiveAddrExpression, {
    // Arrange
    uint8_t mod = Memory_mode_no_disp;
    uint8_t rm = 0b110;
    SegmentedAddress at = Create(0, 0);
    int16_t expDisplacement = 1234;
    WriteWordToMemory((uint16_t)expDisplacement, at);

    EffectiveAddrExpression res = {};

    // Act
    DecodeEffectiveAddrExpression(mod, rm, res, at);

    // Assert
    ASSERT_EQUAL(res.calculationType, Effective_addr_direct_address);
    ASSERT_EQUAL(res.displacement, expDisplacement);
    ASSERT_EQUAL(at.offset, 2);
})

// TEST(DecodeEffectiveAddrExpression_InvalidModRaisesError, {
//     // TODO
// })

// TEST(DecodeEffectiveAddrExpression_InvalidRmRaisesError, {
//     // TODO
// })

// TEST(DecodeEffectiveAddrExpression_InvalidSegmentAddress, {

// })

TEST(InterpretModRm_SuccessfullyInterpretsMemoryMode, {
    // Arrange
    uint8_t mod = Memory_mode_no_disp;
    uint8_t rm = 0b000;
    uint8_t w = WIDE;
    SegmentedAddress at = Create(0, 0);
    Operand res = {};

    // Act
    InterpretModRm(mod, rm, w, res, at);

    // Assert
    ASSERT_EQUAL(res.type, OpType_effectiveAddrCalc);
    ASSERT_EQUAL(res.expression.calculationType, Effective_addr_bx_si);
    ASSERT_EQUAL(res.expression.hasDisplacement, FALSE);
    ASSERT_EQUAL(at.offset, 0);
})

TEST(InterpretModRm_SuccesfullyInterprets8BitDisplacementMode, {
    // Arrange
    uint8_t mod = Memory_mode_8_bit_disp;
    uint8_t rm = 0b000;
    uint8_t w = WIDE;
    SegmentedAddress at = Create(0, 0);
    int8_t expDisplacement = -5;
    WriteByteToMemory((uint8_t)expDisplacement, at);
    Operand res = {};

    // Act
    InterpretModRm(mod, rm, w, res, at);

    // Assert
    ASSERT_EQUAL(res.type, OpType_effectiveAddrCalc);
    ASSERT_EQUAL(res.expression.calculationType, Effective_addr_bx_si);
    ASSERT_EQUAL(res.expression.hasDisplacement, TRUE);
    ASSERT_EQUAL(res.expression.displacement, (int16_t)expDisplacement);
    ASSERT_EQUAL(at.offset, 1);
})

TEST(InterpretModRm_SuccessfullyInterprets16BitDisplacementMode, {
    // Arrange
    uint8_t mod = Memory_mode_16_bit_disp;
    uint8_t rm = 0b000;
    uint8_t w = WIDE;
    SegmentedAddress at = Create(0, 0);
    int16_t expDisplacement = 1234;
    WriteWordToMemory((uint16_t)expDisplacement, at);
    Operand res = {};

    // Act
    InterpretModRm(mod, rm, w, res, at);

    // Assert
    ASSERT_EQUAL(res.type, OpType_effectiveAddrCalc);
    ASSERT_EQUAL(res.expression.calculationType, Effective_addr_bx_si);
    ASSERT_EQUAL(res.expression.hasDisplacement, TRUE);
    ASSERT_EQUAL(res.expression.displacement, expDisplacement);
    ASSERT_EQUAL(at.offset, 2);
})

TEST(InterpretModRm_SuccessfullyInterpretsRegisterMode, {
    // Arrange
    uint8_t mod = Register_mode;
    uint8_t rm = 0b011;
    uint8_t w = WIDE;
    SegmentedAddress at = Create(0, 0);
    Operand res = {};

    // Act
    InterpretModRm(mod, rm, w, res, at);

    // Assert
    ASSERT_EQUAL(res.type, OpType_register);
    ASSERT_EQUAL(res.reg.index, Register_b);
    ASSERT_EQUAL(res.reg.offset, FULL_BITS);
})

TEST(ParseDataFromByte_SuccessfullyParsesDataFromByeUsingBitsWithoutCursorIncrement, {
    // Arrange
    Bits bits = {};
    bits.field = W_bit;
    bits.mask = 0b1;
    bits.shift = 0;
    bits.count = 1;
    uint8_t usedBits = 0;
    SegmentedAddress at = Create(0, 0);
    WriteByteToMemory(0b00000001, at);

    // Act
    uint8_t res = ParseDataFromByte(bits, usedBits, at);

    // Assert
    ASSERT_EQUAL(res, 1);
    ASSERT_EQUAL(at.offset, 0);
})

TEST(ParseDataFromByte_SuccessfullyParsesDataFromByeUsingBitsWithCursorIncrement, {
    // Arrange
    Bits bits = {};
    bits.field = W_bit;
    bits.mask = 0b1;
    bits.shift = 0;
    bits.count = 1;
    uint8_t usedBits = 8;
    SegmentedAddress at = Create(0, 0);
    WriteByteToMemory(0b00000000, at);
    WriteByteToMemory(0b00000001, Create(0, 1));

    // Act
    uint8_t res = ParseDataFromByte(bits, usedBits, at);

    // Assert
    ASSERT_EQUAL(res, 1);
    ASSERT_EQUAL(at.offset, 1);
    ASSERT_EQUAL(usedBits, 0);
})


TESTS(DecodeTests) = {
    DecodeRegister_DecodesWideRegisterSuccessfully,
    DecodeRegister_DecodesByteRegisterSuccessfully,

    DecodeEffectiveAddrExpression_SuccessfullyDecodesRmModIntoEffectiveAddrExpression,
    DecodeEffectiveAddrExpression_SuccessfullyDecodesDirectAddressIntoEffectiveAddrExpression,

    InterpretModRm_SuccessfullyInterpretsMemoryMode,
    InterpretModRm_SuccesfullyInterprets8BitDisplacementMode,
    InterpretModRm_SuccessfullyInterprets16BitDisplacementMode,
    InterpretModRm_SuccessfullyInterpretsRegisterMode,

    ParseDataFromByte_SuccessfullyParsesDataFromByeUsingBitsWithoutCursorIncrement,
    ParseDataFromByte_SuccessfullyParsesDataFromByeUsingBitsWithCursorIncrement,
};