#pragma once
#include "InstructionTable.h"
#include "Hardware.h"

#include <vector>
#include <optional>
#include <stdio.h>


/**
 * @brief Read a 16-bit little-endian word from `memory` starting at address `PC`.
 *
 * The byte at `memory[PC]` is treated as the low byte and the byte at
 * `memory[PC + 1]` is treated as the high byte. These are combined into a
 * 16-bit value `(high_byte << 8) | low_byte` which is returned.
 *
 * Note: `PC` is passed by reference and is modified by this function.
 *
 * @param memory Array representing emulator memory.
 * @param PC Address of the low byte to read.
 * @return 16-bit unsigned integer composed as `(high_byte << 8) | low_byte`.
 * @note Accessing `memory[PC]` or `memory[PC + 1]` out of bounds is undefined
 *       behavior; callers must ensure `PC` and `PC + 1` are valid addresses.
 */
uint16_t loadWordData(struct CPU &cpu)
{
	uint8_t lowByte = cpu.memory[cpu.PC];
	uint16_t highByte = cpu.memory[++cpu.PC];
	uint16_t wideValue = (highByte << 8) | lowByte;
	return wideValue;
}

/**
 * @brief Read an 8-bit byte from `memory` at address `PC` and return it as a 16-bit unsigned integer.
 * @param program 
 * @param programIndex 
 * @return The data loaded from memory converted to an unsigned 8-bit integer
 */
uint16_t loadByteData(struct CPU &cpu)
{
	return static_cast<uint16_t>(cpu.memory[cpu.PC]);
}

uint16_t loadImmediate(uint8_t width, struct CPU &cpu)
{
	cpu.PC++;

	if (width) return loadWordData(cpu);

	return loadByteData(cpu);
}

uint8_t GetMod(uint8_t modMask, struct CPU& cpu)
{
	return (cpu.memory[cpu.PC] & modMask) >> 6;
}

uint8_t GetRm(uint8_t rmMask, struct CPU& cpu)
{
	return (cpu.memory[cpu.PC] & rmMask);
}

uint8_t GetReg(uint8_t regMask, uint8_t regShift, Instruction& instruction, struct CPU& cpu)
{
	return (cpu.memory[cpu.PC] & regMask) >> regShift;
}

void DecodeMod(uint8_t mod, uint8_t rm, Instruction& instruction, struct CPU &cpu)
{
	switch (mod)
	{
	case 0x00:
	{
		// Special case of MOD that indicates direct memory access with no displacement. 
		if (rm == 0x6)
		{
			cpu.PC++;
			sprintf(instruction.rmMnemonic, "[%d]", loadWordData(cpu));
		}
		else
			sprintf(instruction.rmMnemonic, "[%s]", modEffectiveAddressTable.at(rm));
	} break;
	case 0x01:
	{
		cpu.PC++;
		int16_t data = static_cast<int16_t>(loadByteData(cpu));
		sprintf(instruction.rmMnemonic, "[%s + %d]", modEffectiveAddressTable.at(rm), data);
	} break;
	case 0x02:
	{
		cpu.PC++;
		int16_t data = static_cast<int16_t>(loadWordData(cpu));
		sprintf(instruction.rmMnemonic, "[%s + %d]", modEffectiveAddressTable.at(rm), data);
	} break;
	case 0x03:
	{
		sprintf(instruction.rmMnemonic, "%s", getRegister(rm, instruction.width));
	} break;
	}
}


void DecodeTwoByteLogic(Instruction& instruction, InstructionTableEntry& entry, struct CPU& cpu)
{
	struct TwoByteLogicEntry logicEntry = entry.encoding.twoByteLogicEntry;

	// Get width if w bit is present 
	if (logicEntry.wMask != 0)  instruction.width = (cpu.memory[cpu.PC] & logicEntry.wMask);

	// Get direction if d bit is present, direction is always in bit 1 if it is present so we can just shift right by 1 to get the value
	if (logicEntry.dMask != 0) instruction.direction = (cpu.memory[cpu.PC] & logicEntry.dMask) >> 1;

	cpu.PC++;
	if (logicEntry.regMask != 0)
	{
		uint8_t reg = GetReg(logicEntry.regMask, logicEntry.regShift, instruction, cpu);
		sprintf(instruction.regMnemonic, "%s", getRegister(reg, instruction.width));
	}
	uint8_t mod = GetMod(logicEntry.modMask, cpu);
	uint8_t rm = GetRm(logicEntry.rmMask, cpu);
	DecodeMod(mod, rm, instruction, cpu);
}

void DecodeTwoByteLogicImmediate(Instruction& instruction, InstructionTableEntry& entry, struct CPU& cpu)
{
	struct TwoByteLogicImmediateEntry logicImmediateEntry = entry.encoding.twoByteLogicImmediateEntry;

	// Get width if w bit is present 
	if (logicImmediateEntry.wMask != 0) instruction.width = (cpu.memory[cpu.PC] & logicImmediateEntry.wMask);

	cpu.PC++;
	uint8_t mod = GetMod(logicImmediateEntry.modMask, cpu);
	uint8_t rm = GetRm(logicImmediateEntry.rmMask, cpu);
	DecodeMod(mod, rm, instruction, cpu);
	instruction.immediate = loadImmediate(instruction.width, cpu);
}

void DecodeOneByteLogicImmediate(Instruction& instruction, InstructionTableEntry& entry, struct CPU& cpu)
{
	struct OneByteLogicImmediateEntry logicImmediateEntry = entry.encoding.OneByteLogicImmediateEncoding;

	// Get width if w bit is present 
	instruction.width = (cpu.memory[cpu.PC] & logicImmediateEntry.wMask) >> logicImmediateEntry.wShift;

	uint8_t reg = (cpu.memory[cpu.PC] & logicImmediateEntry.regMask);
	sprintf(instruction.regMnemonic, "%s", getRegister(reg, instruction.width));

	instruction.immediate = loadImmediate(instruction.width, cpu);
}

void DecodeAccumulator(Instruction& instruction, InstructionTableEntry& entry, struct CPU& cpu)
{
	struct ThreeByteAccumulatorEntry accumulatorEntry = entry.encoding.threeByteAccumulatorEncoding;
	instruction.direction = accumulatorEntry.direction;
	cpu.PC++;
	loadImmediate(instruction.width, cpu);
	instruction.immediate = loadImmediate(instruction.width, cpu);
}

void Decode(Instruction& instruction, InstructionTableEntry& entry, struct CPU& cpu)
{
	sprintf(instruction.mnemonic, "%s", entry.mnemonic);

	switch (entry.category)
	{
	case ENCODING_TWO_BYTE_LOGIC:
	{
		DecodeTwoByteLogic(instruction, entry, cpu);
	}break;
	case ENCODING_TWO_BYTE_LOGIC_IMMEDIATE:
	{
		DecodeTwoByteLogicImmediate(instruction, entry, cpu);
	}break;
	case ENCODING_ONE_BYTE_LOGIC_IMMEDIATE:
	{
		DecodeOneByteLogicImmediate(instruction, entry, cpu);
	} break;
	case ENCODING_THREE_BYTE_ACCUMULATOR:
	{
		DecodeAccumulator(instruction, entry, cpu);
	}break;
	}
}

std::optional<InstructionTableEntry> decodeOpcode(const uint8_t byte)
{
	int instructionEntry = findInstruction(byte);

	if (instructionEntry < 0) return std::nullopt;

	return std::make_optional(instructionTable.at(instructionEntry));
}