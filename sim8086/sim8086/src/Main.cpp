// sim8086.cpp : Defines the entry point for the application.

#include "Unity.cpp"

#include <cstring>
#include <iostream>

#define EXECUTE_MODE "-e"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "No input file[s] found." << std::endl;
        return 1;
    }

    char* mode = argv[1];
    std::string asmFile = argv[argc - 1];
    struct Program program = LoadProgramIntoMemory(asmFile);

    if (strcmp(mode, EXECUTE_MODE) == 0)
    {
        Execute(program);
    }
    else {
        Disassemble(program);
    }
    
    return 0;
}
