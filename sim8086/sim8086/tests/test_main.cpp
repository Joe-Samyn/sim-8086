
#include "TestDecode.cpp"

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

    printf("%s ........ %d/%d PASSED", name, passed, (int)fixture.size());
}


int main(int argc, char* argv[]) {
    
    printf("-------- Test Results ---------\n");
    RunTests("Decode Tests", DecodeTests);
    return 0;
}
