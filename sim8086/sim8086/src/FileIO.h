#pragma once

#include "InstructionTable.h"

#include <vector>
#include <fstream>
#include <string>



void writeToFile(std::vector<std::string> instructions)
{
    std::ofstream asmFile;
    asmFile.open("result.asm");

    asmFile << "bits 16\n\n";

    for (auto inst : instructions)
    {
        asmFile << inst;
    }

    asmFile.close();
}
