#pragma once

#include "Sim8086.h"

#include <iostream>
#include <fstream>

#define BUFFER_SIZE 1000
#define INST_LENGTH 30

// TODO - Needs to be declared properly in the CPP file. Only IO should be aware of this variable 
static Instruction DecodedInstructions[BUFFER_SIZE];     // String instruction buffer. Holds all ASM instructions to be printed 
// TODO - Needs to be declared properly in the CPP file. Only IO should be aware of this variable 
static uint16_t DecodedInstIndex = 0;

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
void WriteToConsole();
void WriteToConsole(Instruction instruction);
void DisplayRegisterState(CPU cpu);