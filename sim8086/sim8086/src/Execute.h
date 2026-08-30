#pragma once 

#include "Sim8086.h"
#include <cstdint>

void ExecuteMov(CPU &cpu, const Operand &src, const Operand &dest, uint8_t size);
void ExecuteAdd(CPU &cpu, Operand src, Operand dest, uint8_t size, bool useCarry = false);