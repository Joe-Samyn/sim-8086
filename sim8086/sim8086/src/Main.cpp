// sim8086.cpp : Defines the entry point for the application.

#include "Sim8086.cpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <cstdio>



int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "No input file[s] found." << std::endl;
        return 1;
    }

    std::string asmFile = argv[1];
    struct Program program = LoadProgramIntoMemory(asmFile);

    Disassemble(program);

    return 0;
}
