// sim8086.cpp : Defines the entry point for the application.

#include "Unity.cpp"

#include <cstring>
#include <iostream>

#define EXECUTE_MODE "-e"

// TODO: All needs to be fixed. The flags system is not robust. 
int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cout << "No input file[s] found." << std::endl;
        return 1;
    }

    char* mode = argv[1];
    std::string asmFile = argv[argc - 2];
    OutputFile = argv[argc - 1];
    if (strcmp(OutputFile, "") != 0) {
        OpenAsmFile(OutputFile);
    }

    struct Program program = LoadProgramIntoMemory(asmFile);

    if (strcmp(mode, EXECUTE_MODE) == 0)
    {
        Execute(program);
    }
    else {
        Disassemble(program);
    }
    
    CloseAsmFile();

    return 0;
}
