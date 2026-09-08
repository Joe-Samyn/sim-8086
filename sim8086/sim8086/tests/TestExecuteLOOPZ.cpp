#include "TestFixture.h"
#include "Assert.h"
#include "Execute.h"

TEST(ExecuteLoopz_CXDecrementNotZeroZFNotSetNoJumpOccurs, {
    // Arrange
    CPU cpu = {};
    cpu.registers[Register_c] = 3;
    SegmentedAddress at = Create(0, 10);

    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = -4;

    // Act
    ExecuteLoopz(cpu, at, dest);

    // Assert
    ASSERT_EQUAL(0, at.segment);
    ASSERT_EQUAL(10, at.offset);
})

TEST(ExecuteLoopz_CXDecrementEqualsZeroZFSetNoJumpOccurs, {
    // Arrange
    CPU cpu = {};
    cpu.flags |= Zero;
    cpu.registers[Register_c] = 1;
    SegmentedAddress at = Create(0, 10);

    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = -4;

    // Act
    ExecuteLoopz(cpu, at, dest);

    // Assert
    ASSERT_EQUAL(0, at.segment);
    ASSERT_EQUAL(10, at.offset);
})

TEST(ExecuteLoopz_CXDecrementNotEqualsZeroZFSetJumpOccurs, {
    // Arrange
    CPU cpu = {};
    cpu.flags |= Zero;
    cpu.registers[Register_c] = 4;
    SegmentedAddress at = Create(0, 10);

    Operand dest = {};
    dest.type = OpType_jmp;
    dest.displacement = -4;

    // Act
    ExecuteLoopz(cpu, at, dest);

    // Assert
    ASSERT_EQUAL(0, at.segment);
    ASSERT_EQUAL(6, at.offset);
})

TEST(ExecuteLoopz_CXDecrementNotEqualZeroZFSetFlagsPreserved, {
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
    ExecuteLoopz(cpu, at, dest);
    uint16_t resultFlags = cpu.flags;

    // Assert
    ASSERT_EQUAL(expFlags, resultFlags);
})

TESTS(ExecuteLoopzTests) = {
    ExecuteLoopz_CXDecrementNotZeroZFNotSetNoJumpOccurs,
    ExecuteLoopz_CXDecrementEqualsZeroZFSetNoJumpOccurs,
    ExecuteLoopz_CXDecrementNotEqualsZeroZFSetJumpOccurs,
    ExecuteLoopz_CXDecrementNotEqualZeroZFSetFlagsPreserved
};
