#include "TestFixture.h"
#include "Assert.h"
#include "Execute.h"

TEST(ExecutJnz_ZFNotSetBackwardsJump, {
    // Arrange
    CPU cpu = {};
    SegmentedAddress at = Create(0, 8);

    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = -4;

    SegmentedAddress exp = Create(0, 4);

    // Act
    ExecuteJnz(at, dest, cpu.flags & Zero);
    SegmentedAddress result = at;

    // Assert
    ASSERT_EQUAL(exp.segment, result.segment);
    ASSERT_EQUAL(exp.offset, result.offset);
})

TEST(ExecutJnz_ZFNotSetForwardsJump, {
    // Arrange
    CPU cpu = {};
    SegmentedAddress at = Create(0, 8);

    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = 4;

    SegmentedAddress exp = Create(0, 12);

    // Act
    ExecuteJnz(at, dest, cpu.flags & Zero);
    SegmentedAddress result = at;

    // Assert
    ASSERT_EQUAL(exp.segment, result.segment);
    ASSERT_EQUAL(exp.offset, result.offset);
})

TEST(ExecutJnz_ZFSetNoJump, {
    // Arrange
    CPU cpu = {};
    cpu.flags |= Zero;
    SegmentedAddress at = Create(0, 8);

    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = 4;
    
    uint8_t instSize = 2;

    SegmentedAddress exp = at;

    // Act
    ExecuteJnz(at, dest, cpu.flags & Zero);
    SegmentedAddress result = at;

    // Assert
    ASSERT_EQUAL(exp.segment, result.segment);
    ASSERT_EQUAL(exp.offset, result.offset);
})

TESTS(ExecuteJnzTests) = {
    ExecutJnz_ZFNotSetBackwardsJump,
    ExecutJnz_ZFNotSetForwardsJump,
    ExecutJnz_ZFSetNoJump
};
