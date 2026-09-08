#include "TestFixture.h"
#include "Assert.h"
#include "Execute.h"

TEST(ExecuteLoop_CXDecrementNotZeroJumpOccurs, {
    // Arrange
    CPU cpu = {};
    cpu.registers[Register_c] = 3;
    SegmentedAddress at = Create(0, 10);

    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = -4;

    // Act
    ExecuteLoop(cpu, at, dest);

    // Assert
    ASSERT_EQUAL(0, at.segment);
    ASSERT_EQUAL(6, at.offset);
})

TEST(ExecuteLoop_CXDecrementEqualsZeroNoJumpOccurs, {
    // Arrange
    CPU cpu = {};
    cpu.registers[Register_c] = 1;
    SegmentedAddress at = Create(0, 10);

    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = -4;

    // Act
    ExecuteLoop(cpu, at, dest);

    // Assert
    ASSERT_EQUAL(0, at.segment);
    ASSERT_EQUAL(10, at.offset);
})

TEST(ExecuteLoop_CXDecrementNotEqualZeroFlagsPreserved, {
    // Arrange
    CPU cpu = {};
    cpu.flags |= Sign | Overflow | Carry | Zero;
    cpu.registers[Register_c] = 3;
    SegmentedAddress at = Create(0, 10);

    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = -4;

    uint16_t expFlags = Sign | Overflow | Carry | Zero;

    // Act
    ExecuteLoop(cpu, at, dest);
    uint16_t resultFlags = cpu.flags;

    // Assert
    ASSERT_EQUAL(expFlags, resultFlags);
})

TESTS(ExecuteLoopTests) = {
    ExecuteLoop_CXDecrementNotZeroJumpOccurs,
    ExecuteLoop_CXDecrementEqualsZeroNoJumpOccurs,
    ExecuteLoop_CXDecrementNotEqualZeroFlagsPreserved
};
