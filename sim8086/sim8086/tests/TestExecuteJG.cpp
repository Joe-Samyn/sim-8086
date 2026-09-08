#include "TestFixture.h"
#include "Assert.h"
#include "Execute.h"

TEST(ExecuteJg_ZFClearOFEqualsSFJumpOccurs, {
    // Arrange
    uint16_t flags = 0;
    SegmentedAddress at = Create(0, 12);

    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = 4;

    SegmentedAddress exp = Create(0, 16);

    // Act
    ExecuteJg(at, dest, flags);
    SegmentedAddress result = at;

    // Assert
    ASSERT_EQUAL(exp.segment, result.segment);
    ASSERT_EQUAL(exp.offset, result.offset);
})

TEST(ExecuteJg_ZFNotClearOFEqualsSFJumpNotOccur, {
    // Arrange
    uint16_t flags = Zero;
    SegmentedAddress at = Create(0, 12);

    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = 4;

    SegmentedAddress exp = Create(0, 12);

    // Act
    ExecuteJg(at, dest, flags);
    SegmentedAddress result = at;

    // Assert
    ASSERT_EQUAL(exp.segment, result.segment);
    ASSERT_EQUAL(exp.offset, result.offset);
})

TEST(ExecuteJg_ZFClearOFNotEqualsSFJumpNotOccur, {
    // Arrange
    uint16_t flags = Overflow;
    SegmentedAddress at = Create(0, 12);

    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = 4;

    SegmentedAddress exp = Create(0, 12);

    // Act
    ExecuteJg(at, dest, flags);
    SegmentedAddress result = at;

    // Assert
    ASSERT_EQUAL(exp.segment, result.segment);
    ASSERT_EQUAL(exp.offset, result.offset);
})

TESTS(ExecuteJgTests) = {
    ExecuteJg_ZFClearOFEqualsSFJumpOccurs,
    ExecuteJg_ZFNotClearOFEqualsSFJumpNotOccur,
    ExecuteJg_ZFClearOFNotEqualsSFJumpNotOccur
};
