#pragma once

#include "Sim8086.h"

#include <iostream>
#include <fstream>

const char* OutputFile;

enum OutputLocation {
    File = (1 << 0),
    Console = (1 << 1)
};

// TODO - Needs to be declared properly in the CPP file once references outside IO have been fixed.
static const char* RegisterNames[Register_count][2] = {
    {"AL", "AX"},
    {"CL", "CX"},
    {"DL", "DX"},
    {"BL", "BX"},
    {"AH", "SP"},
    {"CH", "BP"},
    {"DH", "SI"},
    {"BH", "DI"}
};

static const char* EAExpressions[EAType_count] = {
    "DIRECT_ADDRESS_ERROR",
    "BX+SI",
    "BX+DI",
    "BP+SI",
    "BP+DI",
    "SI",
    "DI",
    "BP",
    "BX"
};

void OpenAsmFile(std::string name);
void CloseAsmFile();
void DisplayRegisterState(CPU cpu);
void DisplayCpuFlagState(const CPU &cpu);
void WriteInstructionToOutput(const Instruction &instruction, uint8_t outputLocation);
void WriteMemoryToFile(uint8_t* memory);
