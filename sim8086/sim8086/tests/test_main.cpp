
#include <stdio.h>
#include <assert.h>

#include "Sim8086.cpp"


#define DisplaySuccessResult printf("%s..........SUCCESS\n", __func__)
#define DisplayFailureResult printf("%s..........FAIL\n", __func__)

#define AssertEqual(arg1, arg2) do { \
        if (arg1 != arg2) { \
             DisplayFailureResult; \
             return; \
        }\
    } \
    while (0)
    

/* Unit Tests */

void Test_IsBitsDefined_ReturnsFalseWhenNotDefined()
{
    
    Bits bits = {};

    bool result = IsBitsDefined(bits);

    AssertEqual(result, false);
    DisplaySuccessResult;

}

void Test_IsBitsDefined_ReturnsTrueWhenDefined()
{

    Bits bits = {
        .field = W_bit,
        .value = 0x1,
        .shift = 0x1,
        .count = 1
    };

    int result = IsBitsDefined(bits);

    AssertEqual(result, true); 
    DisplaySuccessResult;
}

void Test_GetNextByte_ReturnsByteAndIncrementsIP() 
{
    
    Memory[0] = 1;
    Memory[1] = 2;
    CPU cpu = {};

    uint8_t result = GetNextByte(cpu.IP);

    AssertEqual(result, 1);
    AssertEqual(cpu.IP, 1);

    DisplaySuccessResult;

}

void Test_GetNextWord_ReturnsTwoBytesIncrementsIPByTwo()
{
    Memory[0] = 1;
    Memory[1] = 2;
    Memory[2] = 3;
    CPU cpu = {};
    uint16_t exp = 0b001000000001;
    
    uint16_t result = GetNextWord(cpu.IP);

    AssertEqual(result, exp);
    AssertEqual(cpu.IP, 2);

    DisplaySuccessResult;
}

void Test_DecodeEffectiveAddrExpression_ReturnsBxPlusSi()
{
    uint8_t mod = 0;
    uint8_t rm = 0;
    EffectiveAddrExpression result = {};
    CPU cpu = {};

    DecodeEffectiveAddrExpression(mod, rm, result, cpu);

    AssertEqual(result.calculationType, Effective_addr_bx_si);
    AssertEqual(result.base.index, Register_b);
    AssertEqual(result.base.offset, FULL_BITS);
    AssertEqual(result.index.index, Register_si);
    AssertEqual(result.index.offset, FULL_BITS);

    DisplaySuccessResult;

}

/* End-To-End Tests */

void Test_Decode_DecodesRegToRegMovSuccessfully()
{
    Memory[0] = 0x89;
    Memory[1] = 0xd8;
    CPU cpu = {.IP = 1};
    Entry entry = InstructionTable[0];

    Instruction result = Decode(cpu, entry);

    AssertEqual(result.op, Op_MOV);
    AssertEqual(result.w, 1);
    AssertEqual(result.d, 0);


    DisplaySuccessResult;

}


int main(int argc, char* argv[]) {
    
    printf("-------- Test Resuts ---------\n\n");
    
    Test_IsBitsDefined_ReturnsFalseWhenNotDefined();
    Test_IsBitsDefined_ReturnsTrueWhenDefined();
    Test_GetNextByte_ReturnsByteAndIncrementsIP(); 
    Test_GetNextWord_ReturnsTwoBytesIncrementsIPByTwo();
    Test_DecodeEffectiveAddrExpression_ReturnsBxPlusSi();
    Test_Decode_DecodesRegToRegMovSuccessfully();

    printf("\n-------- End Tests --------\n");
    return 0;
}
