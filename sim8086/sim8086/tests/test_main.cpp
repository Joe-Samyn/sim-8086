
#include <stdio.h>
#include <assert.h>

#include "Sim8086.cpp"


#define DisplaySuccessResult(funcName) printf("%s..........SUCCESS\n", funcName);

void Test_IsBitsDefined_ReturnsFalseWhenNotDefined()
{
    
    Bits bits = {};

    bool result = IsBitsDefined(bits);

    assert(result == false);

    DisplaySuccessResult(__func__);
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

    assert(result == true);
    
    DisplaySuccessResult(__func__);
}

int main(int argc, char* argv[]) {
    
    printf("-------- Running Tests ---------\n");
    
    Test_IsBitsDefined_ReturnsFalseWhenNotDefined();
    Test_IsBitsDefined_ReturnsTrueWhenDefined();
    printf("-------- End Tests --------\n");
    return 0;
}
