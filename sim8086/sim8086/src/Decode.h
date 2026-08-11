#pragma once 

#include "Sim8086.h"

#include <cstdint>


Instruction Decode(Entry entry, SegmentedAddress &at);

uint8_t ParseDataFromByte(Bits current, uint8_t &usedBits, SegmentedAddress &cursor);
void InterpretModRm(uint8_t mod, uint8_t rm, uint8_t w,  Operand &operand, SegmentedAddress &at);
void DecodeEffectiveAddrExpression(uint8_t mod, uint8_t rm, EffectiveAddrExpression &expression, SegmentedAddress &at);
void DecodeRegister(uint8_t reg, uint8_t w, RegisterAccess &regAccess);