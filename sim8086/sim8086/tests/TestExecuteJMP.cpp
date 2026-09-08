#include "TestFixture.h"
#include "Assert.h"
#include "Execute.h"

/*
    NOTE: Just validating in segment jumps right now. Out of segment support & tests will come later.
*/

TEST(ExecuteJmp_JumpsForwardToCorrectAddress, {
    // Arrange
    SegmentedAddress at = Create(0, 2);

    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = 4;

    SegmentedAddress exp = Create(0, 6);

    // Act
    ExecuteJmp(at, dest);
    SegmentedAddress result = at;

    // Assert
    ASSERT_EQUAL(exp.segment, result.segment);
    ASSERT_EQUAL(exp.offset, result.offset);
})

TEST(ExecuteJmp_JumpsBackwardsToCorrectAddress, {
    // Arrange
    SegmentedAddress at = Create(0, 6);

    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = -2;

    SegmentedAddress exp = Create(0, 4);

    // Act
    ExecuteJmp(at, dest);
    SegmentedAddress result = at;

    // Assert
    ASSERT_EQUAL(exp.segment, result.segment);
    ASSERT_EQUAL(exp.offset, result.offset);
})

TESTS(ExecuteJmpTests) = {
    ExecuteJmp_JumpsForwardToCorrectAddress,
    ExecuteJmp_JumpsBackwardsToCorrectAddress
};
