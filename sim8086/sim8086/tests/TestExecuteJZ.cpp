#include "TestFixture.h"
#include "Assert.h"
#include "Execute.h"

TEST(ExecutJz_ZFSetBackwardsJump, {
    // Arrange
    CPU cpu = {};
    cpu.flags |= Zero;
    SegmentedAddress at = Create(0, 8);

    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = -4;

    SegmentedAddress exp = Create(0, 4);

    // Act
    ExecuteJz(at, dest, cpu.flags & Zero);
    SegmentedAddress result = at;

    // Assert
    ASSERT_EQUAL(exp.segment, result.segment);
    ASSERT_EQUAL(exp.offset, result.offset);
})

TEST(ExecutJz_ZFSetForwardsJump, {
    // Arrange
    CPU cpu = {};
    cpu.flags |= Zero;
    SegmentedAddress at = Create(0, 8);

    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = 4;

    SegmentedAddress exp = Create(0, 12);

    // Act
    ExecuteJz(at, dest, cpu.flags & Zero);
    SegmentedAddress result = at;

    // Assert
    ASSERT_EQUAL(exp.segment, result.segment);
    ASSERT_EQUAL(exp.offset, result.offset);
})

TEST(ExecutJz_ZFNotSetNoJump, {
    // Arrange
    CPU cpu = {};
    SegmentedAddress at = Create(0, 8);

    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = 4;
    
    uint8_t instSize = 2;

    SegmentedAddress exp = at;

    // Act
    ExecuteJz(at, dest, cpu.flags & Zero);
    SegmentedAddress result = at;

    // Assert
    ASSERT_EQUAL(exp.segment, result.segment);
    ASSERT_EQUAL(exp.offset, result.offset);
})

TESTS(ExecuteJzTests) = {
    ExecutJz_ZFSetBackwardsJump,
    ExecutJz_ZFSetForwardsJump,
    ExecutJz_ZFNotSetNoJump
};
