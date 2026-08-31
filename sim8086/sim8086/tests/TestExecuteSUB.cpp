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

TESTS(ExecuteSubTests) = {
    ExecuteSub_FullRegisterFromFullRegisterNoFlagsSet,
    ExecuteAdd_RegisterLoByteFromLoByteNoFlagsSet
};