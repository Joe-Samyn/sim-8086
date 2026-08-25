#pragma once

#include "Sim8086.h"

#include <iostream>
#include <fstream>

enum OutputLocation {
    File = (1 << 0),
    Console = (1 << 1)
};

// TODO - Needs to be declared properly in the CPP file once references outside IO have been fixed. 
static const char* RegisterNames[Register_count][3] = {
    {"AL", "AH", "AX"},
    {"BL", "BH", "BX"},
    {"CL", "CH", "CX"},
    {"DL", "DH", "DX"},
    {"", "", "SP"},
    {"", "", "BP"},
    {"", "", "SI"},
    {"", "", "DI"}
};

std::ofstream OpenAsmFile(std::string name);
void CloseAsmFile(std::ofstream &file);
void DisplayRegisterState(CPU cpu);
void DisplayCpuFlagState(const CPU &cpu);
void WriteInstructionToOutput(const Instruction &instruction, uint8_t outputLocation);