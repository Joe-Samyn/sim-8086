#pragma once 

#include "Sim8086.h"
#include <cstdint>

void ExecuteMov(CPU &cpu, const Operand &src, const Operand &dest, uint8_t size);
void ExecuteAdd(CPU &cpu, Operand src, Operand dest, uint8_t size, bool useCarry = false);
void ExecuteSub(CPU &cpu, Operand src, Operand dest, uint8_t size, bool useCarry = false);
void ExecuteCmp(CPU &cpu, Operand src, Operand dest, uint8_t size);
void ExecuteJmp(SegmentedAddress &at, const Operand &dest);
void ExecuteJnz(SegmentedAddress &at, const Operand &dest, uint16_t zf);
void ExecuteJz(SegmentedAddress &at, const Operand &dest, uint16_t zf);
void ExecuteJg(SegmentedAddress &at, const Operand &dest, uint16_t flags);
void ExecuteJge(SegmentedAddress &at, const Operand &dest, uint16_t flags);
void ExecuteJl(SegmentedAddress &at, const Operand &dest, uint16_t flags);
void ExecuteJng(SegmentedAddress &at, const Operand &dest, uint16_t flags);
