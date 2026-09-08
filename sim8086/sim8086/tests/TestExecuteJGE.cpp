#include "TestFixture.h"
#include "Assert.h"
#include "Execute.h"

TEST(ExecuteJge_ZFSetOFEqualsSFJumpOccurs, {
    // ARRANGE
    SegmentedAddress at = Create(0, 10);
    uint16_t flags = Zero;

    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = 4;

    SegmentedAddress exp = Create(0, 14);

    // ACT
    ExecuteJge(at, dest, flags);
    SegmentedAddress result = at;

    // ASSERT
    ASSERT_EQUAL(exp.segment, result.segment);
    ASSERT_EQUAL(exp.offset, result.offset);
})

TESTS(ExecuteJgeTests) = {
    ExecuteJge_ZFSetOFEqualsSFJumpOccurs
};
