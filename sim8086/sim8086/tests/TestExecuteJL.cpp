#include "TestFixture.h"
#include "Assert.h"
#include "Execute.h"

TEST(ExecuteJl_SFEqualsOFNoJumpOccurs, {
    // ARRANGE
    SegmentedAddress at = Create(2, 10);
    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = -4;
    uint16_t flags = 0;

    SegmentedAddress exp = Create(2, 10);

    // ACT
    ExecuteJl(at, dest, flags);
    SegmentedAddress result = at;

    // ASSERT
    ASSERT_EQUAL(exp.segment, result.segment);
    ASSERT_EQUAL(exp.offset, result.offset);
})

TEST(ExecuteJl_SFNotEqualOFJumpOccurs, {
    // ARRANGE
    SegmentedAddress at = Create(2, 10);
    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = -4;
    uint16_t flags = Overflow;

    SegmentedAddress exp = Create(2, 6);

    // ACT
    ExecuteJl(at, dest, flags);
    SegmentedAddress result = at;

    // ASSERT
    ASSERT_EQUAL(exp.segment, result.segment);
    ASSERT_EQUAL(exp.offset, result.offset);
})

TESTS(ExecuteJlTests) = {
    ExecuteJl_SFEqualsOFNoJumpOccurs,
    ExecuteJl_SFNotEqualOFJumpOccurs
};
