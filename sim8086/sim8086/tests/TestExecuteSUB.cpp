#include "Assert.h"
#include "TestFixture.h"

#include "Sim8086.h"
#include "Execute.h"

TEST(ExecuteSub_FullRegisterFromFullRegisterNoFlagsSet, {
    // ARRANGE 
   CPU cpu = {};
   cpu.registers[Register_b] = 0x5;
   cpu.registers[Register_c] = 0x2;

   uint8_t size = WIDE;
   bool useCarry = false;

   Operand dest = {};
   dest.type = OpType_register;
   dest.reg.index = Register_b;
   dest.reg.offset = FULL_BITS;

   Operand src = {};
   src.type = OpType_register;
   src.reg.index = Register_c;
   src.reg.offset = FULL_BITS;

   uint16_t exp = 0x0003;


    // ACT
    ExecuteSub(cpu, src, dest, size, useCarry);
    uint16_t result = cpu.registers[Register_b];
    uint16_t flags = cpu.flags;

    // ASSERT
    ASSERT_EQUAL(result, exp);
    ASSERT_EQUAL(flags, 0);
})

TEST(ExecuteAdd_RegisterLoByteFromLoByteNoFlagsSet, {
     // ARRANGE 
   CPU cpu = {};
   cpu.registers[Register_b] = 0xAB05;
   cpu.registers[Register_c] = 0xCD02;

   uint8_t size = BYTE;
   bool useCarry = false;

   Operand dest = {};
   dest.type = OpType_register;
   dest.reg.index = Register_b;
   dest.reg.offset = LO_BITS;

   Operand src = {};
   src.type = OpType_register;
   src.reg.index = Register_c;
   src.reg.offset = LO_BITS;

   uint16_t exp = 0xAB03;
   uint8_t expLo = 0x03;


    // ACT
    ExecuteSub(cpu, src, dest, size, useCarry);
    uint16_t result = cpu.registers[Register_b];
    uint8_t resultLo = GetLowByte(cpu.registers[Register_b]);
    uint16_t flags = cpu.flags;

    // ASSERT
    ASSERT_EQUAL(result, exp);
    ASSERT_EQUAL(resultLo, expLo);
    ASSERT_EQUAL(flags, 0);
})

TEST(ExecuteSub_FullRegFromFullRegCFSet, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0x2244;
    cpu.registers[Register_c] = 0x8811;

    uint8_t size = WIDE;
    bool useCarry = false;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_b;
    dest.reg.offset = FULL_BITS;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_c;
    src.reg.offset = FULL_BITS;

    uint16_t exp = 0x9A33;

    // ACT
    ExecuteSub(cpu, src, dest, size, useCarry);
    uint16_t result = cpu.registers[Register_b];
    uint16_t cfResult = cpu.flags & Carry;

    // ASSERT
    ASSERT_EQUAL(result, exp);
    ASSERT_EQUAL(cfResult, Carry);
})

TEST(ExecuteSub_FullRegFromFullRegOFSet, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0x8001;
    cpu.registers[Register_c] = 0x43AB;

    uint8_t size = WIDE;
    bool useCarry = false;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_b;
    dest.reg.offset = FULL_BITS;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_c;
    src.reg.offset = FULL_BITS;

    uint16_t exp = 0x3C56;

    // ACT
    ExecuteSub(cpu, src, dest, size, useCarry);
    uint16_t result = cpu.registers[Register_b];
    uint16_t ofResult = cpu.flags & Overflow;

    // ASSERT
    ASSERT_EQUAL(result, exp);
    ASSERT_EQUAL(ofResult, Overflow);
})

TEST(ExecuteSub_FullRegisterFromFullRegisterSFSet, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0xFF04;
    cpu.registers[Register_c] = 0x0003;

    uint8_t size = WIDE;
    bool useCarry = false;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_b;
    dest.reg.offset = FULL_BITS;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_c;
    src.reg.offset = FULL_BITS;

    uint16_t exp = 0xFF01;

    // ACT
    ExecuteSub(cpu, src, dest, size, useCarry);
    uint16_t result = cpu.registers[Register_b];
    uint16_t sfResult = cpu.flags & Sign;

    // ASSERT
    ASSERT_EQUAL(result, exp);
    ASSERT_EQUAL(sfResult, Sign);
})

TEST(ExecuteSub_FullRegisterFromFullRegisterZFSet, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0xFF01;
    cpu.registers[Register_c] = 0xFF01;

    uint8_t size = WIDE;
    bool useCarry = false;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_b;
    dest.reg.offset = FULL_BITS;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_c;
    src.reg.offset = FULL_BITS;

    uint16_t exp = 0;

    // ACT
    ExecuteSub(cpu, src, dest, size, useCarry);
    uint16_t result = cpu.registers[Register_b];
    uint16_t zfResult = cpu.flags & Zero;

    // ASSERT
    ASSERT_EQUAL(result, exp);
    ASSERT_EQUAL(zfResult, Zero);
})

TEST(ExecuteSub_LoByteRegFromHiByteRegNoFlags, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0x08FF;
    cpu.registers[Register_c] = 0xAB03;

    uint8_t size = BYTE;
    bool useCarry = false;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_b;
    dest.reg.offset = HI_BITS;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_c;
    src.reg.offset = LO_BITS;

    uint16_t exp = 0x05FF;
    uint8_t expHi = 0x05;

    // ACT
    ExecuteSub(cpu, src, dest, size, useCarry);
    uint16_t result = cpu.registers[Register_b];
    uint8_t resultHi = ReadHiByte(cpu.registers[Register_b]);
    uint16_t flags = cpu.flags;

    // ASSERT
    ASSERT_EQUAL(result, exp);
    ASSERT_EQUAL(resultHi, expHi);
    ASSERT_EQUAL(flags, 0);
})

TEST(ExecuteSub_HiByteRegFromLoByteReg, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0xFF08;
    cpu.registers[Register_c] = 0x03AB;

    uint8_t size = BYTE;
    bool useCarry = false;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_b;
    dest.reg.offset = LO_BITS;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_c;
    src.reg.offset = HI_BITS;

    uint16_t exp = 0xFF05;
    uint8_t expLo = 0x05;

    // ACT
    ExecuteSub(cpu, src, dest, size, useCarry);
    uint16_t result = cpu.registers[Register_b];
    uint8_t resultLo = ReadLoByte(cpu.registers[Register_b]);
    uint16_t flags = cpu.flags;

    // ASSERT
    ASSERT_EQUAL(result, exp);
    ASSERT_EQUAL(resultLo, expLo);
    ASSERT_EQUAL(flags, 0);
})

TEST(ExecuteSub_FullRegFromMemoryNoFlags, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0x0120;
    cpu.registers[Register_si] = 0x0020;
    cpu.registers[Register_c] = 0x0005;

    SegmentedAddress at = Create(cpu.segmentRegisters[DS], 0x0140);
    WriteWordToMemory(0x0008, at);

    uint8_t size = WIDE;
    bool useCarry = false;

    Operand dest = {};
    dest.type = OpType_effectiveAddrCalc;
    dest.expression.calculationType = Effective_addr_bx_si;
    dest.expression.base.index = Register_b;
    dest.expression.index.index = Register_si;
    dest.expression.hasDisplacement = FALSE;
    dest.expression.displacement = 0;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_c;
    src.reg.offset = FULL_BITS;

    uint16_t exp = 0x0003;

    // ACT
    ExecuteSub(cpu, src, dest, size, useCarry);
    uint16_t result = ReadWordFromMemory(at);

    // ASSERT
    ASSERT_EQUAL(result, exp);
})

TEST(ExecuteSub_MemoryFromFullRegNoFlags, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0x0120;
    cpu.registers[Register_si] = 0x0020;
    cpu.registers[Register_c] = 0x0008;

    SegmentedAddress at = Create(cpu.segmentRegisters[DS], 0x0140);
    WriteWordToMemory(0x0005, at);

    uint8_t size = WIDE;
    bool useCarry = false;

    Operand src = {};
    src.type = OpType_effectiveAddrCalc;
    src.expression.calculationType = Effective_addr_bx_si;
    src.expression.base.index = Register_b;
    src.expression.index.index = Register_si;
    src.expression.hasDisplacement = FALSE;
    src.expression.displacement = 0;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_c;
    dest.reg.offset = FULL_BITS;

    uint16_t exp = 0x0003;

    // ACT
    ExecuteSub(cpu, src, dest, size, useCarry);
    uint16_t result = cpu.registers[Register_c];

    // ASSERT
    ASSERT_EQUAL(result, exp);
})

TEST(ExecuteSub_LoByteFromMemoryNoFlags, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0x0120;
    cpu.registers[Register_si] = 0x0020;
    cpu.registers[Register_c] = 0xFF05;

    SegmentedAddress at = Create(cpu.segmentRegisters[DS], 0x0140);
    WriteWordToMemory(0xAB08, at);

    uint8_t size = BYTE;
    bool useCarry = false;

    Operand dest = {};
    dest.type = OpType_effectiveAddrCalc;
    dest.expression.calculationType = Effective_addr_bx_si;
    dest.expression.base.index = Register_b;
    dest.expression.index.index = Register_si;
    dest.expression.hasDisplacement = FALSE;
    dest.expression.displacement = 0;

    Operand src = {};
    src.type = OpType_register;
    src.reg.index = Register_c;
    src.reg.offset = LO_BITS;

    uint16_t exp = 0xAB03;
    uint8_t expLo = 0x03;

    // ACT
    ExecuteSub(cpu, src, dest, size, useCarry);
    uint16_t result = ReadWordFromMemory(at);
    uint8_t resultLo = ReadByteFromMemory(at);

    // ASSERT
    ASSERT_EQUAL(result, exp);
    ASSERT_EQUAL(resultLo, expLo);
})

TEST(ExecuteSub_MemoryFromLoByteNoFlags, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0x0120;
    cpu.registers[Register_si] = 0x0020;
    cpu.registers[Register_c] = 0xFF08;

    SegmentedAddress at = Create(cpu.segmentRegisters[DS], 0x0140);
    WriteWordToMemory(0xAB05, at);

    uint8_t size = BYTE;
    bool useCarry = false;

    Operand src = {};
    src.type = OpType_effectiveAddrCalc;
    src.expression.calculationType = Effective_addr_bx_si;
    src.expression.base.index = Register_b;
    src.expression.index.index = Register_si;
    src.expression.hasDisplacement = FALSE;
    src.expression.displacement = 0;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_c;
    dest.reg.offset = LO_BITS;

    uint16_t exp = 0xFF03;
    uint8_t expLo = 0x03;

    // ACT
    ExecuteSub(cpu, src, dest, size, useCarry);
    uint16_t result = cpu.registers[Register_c];
    uint8_t resultLo = ReadLoByte(cpu.registers[Register_c]);

    // ASSERT
    ASSERT_EQUAL(result, exp);
    ASSERT_EQUAL(resultLo, expLo);
})

TEST(ExecuteSub_ImmediateFromFullRegisterNoFlags, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0x2467;

    uint8_t size = WIDE;
    bool useCarry = false;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_b;
    dest.reg.offset = FULL_BITS;

    Operand src = {};
    src.type = OpType_immediate;
    src.immediate = 0x0132;

    uint16_t exp = 0x2335;

    // ACT
    ExecuteSub(cpu, src, dest, size, useCarry);
    uint16_t result = cpu.registers[Register_b];
    uint16_t flags = cpu.flags;

    // ASSERT
    ASSERT_EQUAL(result, exp);
    ASSERT_EQUAL(flags, 0);
})

TEST(ExecuteSub_ImmediateFromLoByteNoFlags, {
    // ARRANGE
    CPU cpu = {};
    cpu.registers[Register_b] = 0x2467;

    uint8_t size = BYTE;
    bool useCarry = false;

    Operand dest = {};
    dest.type = OpType_register;
    dest.reg.index = Register_b;
    dest.reg.offset = FULL_BITS;

    Operand src = {};
    src.type = OpType_immediate;
    src.immediate = 0x32;

    uint16_t exp = 0x2435;
    uint8_t expLo = 0x35;

    // ACT
    ExecuteSub(cpu, src, dest, size, useCarry);
    uint16_t result = cpu.registers[Register_b];
    uint8_t resultLo = ReadLoByte(cpu.registers[Register_b]);
    uint16_t flags = cpu.flags;

    // ASSERT
    ASSERT_EQUAL(result, exp);
    ASSERT_EQUAL(resultLo, expLo);
    ASSERT_EQUAL(flags, 0);
})

TESTS(ExecuteSubTests) = {
    ExecuteSub_FullRegisterFromFullRegisterNoFlagsSet,
    ExecuteAdd_RegisterLoByteFromLoByteNoFlagsSet,
    ExecuteSub_FullRegFromFullRegCFSet,
    ExecuteSub_FullRegFromFullRegOFSet,
    ExecuteSub_FullRegisterFromFullRegisterSFSet,
    ExecuteSub_FullRegisterFromFullRegisterZFSet,
    ExecuteSub_LoByteRegFromHiByteRegNoFlags,
    ExecuteSub_HiByteRegFromLoByteReg,
    ExecuteSub_FullRegFromMemoryNoFlags,
    ExecuteSub_MemoryFromFullRegNoFlags,
    ExecuteSub_LoByteFromMemoryNoFlags,
    ExecuteSub_MemoryFromLoByteNoFlags,
    ExecuteSub_ImmediateFromFullRegisterNoFlags,
    ExecuteSub_ImmediateFromLoByteNoFlags
};