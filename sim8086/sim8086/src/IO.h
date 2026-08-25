#pragma once

#include "Sim8086.h"

#include <iostream>
#include <fstream>

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
void PrintEffectiveAddressExpression(Operand op);
void PrintOperand(Operand op);
void WriteToFile();
void WriteToConsole(Instruction instruction);
void DisplayRegisterState(CPU cpu);
void DisplayCpuFlagState(const CPU &cpu);