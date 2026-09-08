
#include "Unity.cpp"
#include "TestDecode.cpp"
#include "TestExecuteMOV.cpp"
#include "TestExecuteADD.cpp"
#include "TestExecuteADC.cpp"
#include "TestExecuteSUB.cpp"
#include "TestExecuteSBB.cpp"
#include "TestExecuteCMP.cpp"
#include "TestExecuteJNZ.cpp"
#include "TestExecuteJZ.cpp"
#include "TestExecuteJMP.cpp"
#include "TestExecuteJG.cpp"
#include "TestExecuteJGE.cpp"
#include "TestExecuteJL.cpp"
#include "TestExecuteJNG.cpp"

#include <cstdio>

static void RunTests(const char* name, const TestArray &fixture) {
    int passed = 0;
    for (int i = 0; i < fixture.size(); i++) {
        TestResult testResult = fixture.at(i)();
        if (testResult.result < 0) {
            printf("%s ......... FAILED\n", testResult.name);
        }
        else {
            printf("%s ......... PASSED\n", testResult.name);
            ++passed;
        }
    }

    printf("%s ........ %d/%d PASSED\n\n", name, passed, (int)fixture.size());
}


int main(int argc, char* argv[]) {
    
    printf("-------- Test Results ---------\n");
    RunTests("Decode Tests", DecodeTests);
    RunTests("Execute MOV Tests", ExecuteMovTests);
    RunTests("Execute ADD Tests", ExecuteAddTests);
    RunTests("Execute ADC Tests", ExecuteAdcTests);
    RunTests("Execute SUB Tests", ExecuteSubTests);
    RunTests("Execute SBB Tests", ExecuteSbbTests);
    RunTests("Execute CMP Tests", ExecuteCmpTests);
    RunTests("Execute JNZ Tests", ExecuteJnzTests);
    RunTests("Execute JZ Tests", ExecuteJzTests);
    RunTests("Execute JMP Tests", ExecuteJmpTests);
    RunTests("Execute JG Tests", ExecuteJgTests);
    RunTests("Execute JGE Tests", ExecuteJgeTests);
    RunTests("Execute JL Tests", ExecuteJlTests);
    RunTests("Execute JNG Tests", ExecuteJngTests);
    return 0;
}
