#include "TestFixture.h"
#include "Assert.h"
#include "Execute.h"

TEST(ExecuteJng_OFNotEqualSFAndZFNotSetJumpOccurs, {
    // Arrange
    SegmentedAddress at = Create(0, 10);
    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = -4;
    uint16_t flags = Sign;

    SegmentedAddress exp = Create(0, 6);

    // ACT
    ExecuteJng(at, dest, flags);
    SegmentedAddress result = at;

    // ASSERT
    ASSERT_EQUAL(exp.segment, result.segment);
    ASSERT_EQUAL(exp.offset, result.offset);
})

TEST(ExecuteJng_OFEqualSFAndZFSetJumpOccurs, {
    // Arrange
    SegmentedAddress at = Create(0, 10);
    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = -4;
    uint16_t flags = Zero;

    SegmentedAddress exp = Create(0, 6);

    // ACT
    ExecuteJng(at, dest, flags);
    SegmentedAddress result = at;

    // ASSERT
    ASSERT_EQUAL(exp.segment, result.segment);
    ASSERT_EQUAL(exp.offset, result.offset);
})

TEST(ExecuteJng_OFAndSFSetAndZFNotSetNoJumpOccurs, {
    // Arrange
    SegmentedAddress at = Create(0, 10);
    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = -4;
    uint16_t flags = Sign | Overflow;

    SegmentedAddress exp = Create(0, 10);

    // ACT
    ExecuteJng(at, dest, flags);
    SegmentedAddress result = at;

    // ASSERT
    ASSERT_EQUAL(exp.segment, result.segment);
    ASSERT_EQUAL(exp.offset, result.offset);
})

TEST(ExecuteJng_OFEqualSFAndZFNotSetNoJumpOccurs, {
    // Arrange
    SegmentedAddress at = Create(0, 10);
    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = -4;
    uint16_t flags = 0;

    SegmentedAddress exp = Create(0, 10);

    // ACT
    ExecuteJng(at, dest, flags);
    SegmentedAddress result = at;

    // ASSERT
    ASSERT_EQUAL(exp.segment, result.segment);
    ASSERT_EQUAL(exp.offset, result.offset);
})

TESTS(ExecuteJngTests) = {
    ExecuteJng_OFNotEqualSFAndZFNotSetJumpOccurs,
    ExecuteJng_OFEqualSFAndZFSetJumpOccurs,
    ExecuteJng_OFAndSFSetAndZFNotSetNoJumpOccurs,
    ExecuteJng_OFEqualSFAndZFNotSetNoJumpOccurs
};
