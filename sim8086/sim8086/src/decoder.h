#pragma once

#include <vector>
#include "InstructionTable.h"

/**
 * @brief Decode 2 bytes of data from the program
 * @param program 
 * @param programIndex 
 * @return 
 */
uint16_t loadWideData(std::vector<uint8_t>& program, int& programIndex)
{
	programIndex++;
	uint8_t lowByte = program.at(programIndex);
	programIndex++;
	uint16_t highByte = program.at(programIndex);
	uint16_t wideValue = (highByte << 8) | lowByte;
	return wideValue;
}

/**
 * @brief Decode an Intel 8086 one byte instruction
 * @param instruction The instruction struct to hold decoded data
 * @param entry The instruction entry in the Intel 8086 instruction table
 * @param program Data structure representing the encoded program data (array or vector or bytes)
 * @param programIndex Current index into the program
 */
void decodeOneByteInstruction(Instruction& instruction, InstructionEntry& entry, std::vector<uint8_t>& program, int& programIndex)
{
	// Check if register encoding present, if not it could be an accumulator instruction 
	if (entry.regMask)
	{
		instruction.reg = (program.at(programIndex) & entry.regMask);
	}

	// Check if instruction is a data operation and what size data is being operated on
	if (entry.wMask != 0)
	{
		if (instruction.width == 0)// Load single byte
		{
			programIndex++;
			instruction.immediate = program.at(programIndex);
			snprintf(instruction.regMnemonic, BUFFER_SIZE, "%s", registerTable.at(instruction.reg));
		}
		else
		{
			// load wide bytes
			instruction.immediate = loadWideData(program, programIndex);
			snprintf(instruction.regMnemonic, BUFFER_SIZE, "%s", registerWideTable.at(instruction.reg));
		}
	}
}

/**
 * @brief Decode an Intel 8086 two byte instruction 
 * @param instruction The instruction struct to hold decoded data
 * @param entry The instruction entry in the Intel 8086 instruction table
 * @param program Data structure representing the encoded program data (array or vector or bytes)
 * @param programIndex Current index into the program
 */
void decodeTwoByteInstruction(Instruction& instruction, InstructionEntry& entry, std::vector<uint8_t> program, int& programIndex)
{
	if (entry.modMask != 0)
	{
		programIndex++;
		uint8_t mod = program.at(programIndex) & entry.modMask;

		// TODO: Most of the logic for determing reg, rm, etc. seems to be the same for MOD != 11. Only difference is displacement
		// There should be a way to reuse this logic. 
		switch (mod)
		{
			// Memory mode, no displacement
		case 0b00000000:
		{

			instruction.rm = entry.rmMask & program.at(programIndex);
			instruction.reg = (entry.regMask & program.at(programIndex)) >> 3;
			const char* reg = instruction.width ? registerWideTable.at(instruction.reg) : registerTable.at(instruction.reg);
			snprintf(instruction.regMnemonic, BUFFER_SIZE, "%s", reg);

			// Handle direct address loading 
			if (instruction.rm == 0b00000110)
			{
				uint16_t directAddr = loadWideData(program, programIndex);
				snprintf(instruction.rmMnemonic, BUFFER_SIZE, "[%d]", directAddr);
			}
			else {
				EffectiveAddrCalculation calc = modEffectiveAddressTable.at(instruction.rm);
				// NOTE: When decoding for running a program, we would perform calculation here? 
				// Get register info
				snprintf(instruction.rmMnemonic, BUFFER_SIZE, "[%s]", calc.calcLiteral);
			}

		} break;

		// Memory mode, 8-bit displacement
		case 0b01000000:
		{
			instruction.rm = entry.rmMask & program.at(programIndex);
			EffectiveAddrCalculation calc = modEffectiveAddressTable.at(instruction.rm);

			// NOTE: When decoding for running a program, we would perform calculation here? 
			instruction.reg = (entry.regMask & program.at(programIndex)) >> 3;
			const char* reg = instruction.width ? registerWideTable.at(instruction.reg) : registerTable.at(instruction.reg);
			snprintf(instruction.regMnemonic, BUFFER_SIZE, "%s", reg);

			// Get displacement bytes
			programIndex++;
			int8_t displacement = static_cast<int8_t>(program.at(programIndex));

			// Get register info
			if (displacement < 0)
			{
				displacement = (~displacement) + 1;
				snprintf(instruction.rmMnemonic, BUFFER_SIZE, "[%s - %d]", calc.calcLiteral, displacement);
			}
			else
			{
				snprintf(instruction.rmMnemonic, BUFFER_SIZE, "[%s + %d]", calc.calcLiteral, displacement);
			}
		} break;

		// Memory mode, 16-bit displacement
		case 0b10000000:
		{
			instruction.rm = entry.rmMask & program.at(programIndex);
			EffectiveAddrCalculation calc = modEffectiveAddressTable.at(instruction.rm);

			// NOTE: When decoding for running a program, we would perform calculation here? 
			instruction.reg = (entry.regMask & program.at(programIndex)) >> 3;
			const char* reg = instruction.width ? registerWideTable.at(instruction.reg) : registerTable.at(instruction.reg);
			snprintf(instruction.regMnemonic, BUFFER_SIZE, "%s", reg);

			// Get displacement bytes
			int16_t displacement = static_cast<int16_t>(loadWideData(program, programIndex));

			// Get register info
			if (displacement < 0)
			{
				displacement = (~displacement) + 1;
				snprintf(instruction.rmMnemonic, BUFFER_SIZE, "[%s - %d]", calc.calcLiteral, displacement);
			}
			else
			{
				snprintf(instruction.rmMnemonic, BUFFER_SIZE, "[%s + %d]", calc.calcLiteral, displacement);
			}
		} break;

		// Register mode, no displacement
		case 0b11000000:
		{

			// Both operands are in registers, so decode the registers involved in the operation
			instruction.reg = (program.at(programIndex) & entry.regMask) >> 3;
			instruction.rm = program.at(programIndex) & entry.rmMask;
			const char* reg = instruction.width ? registerWideTable.at(instruction.reg) : registerTable.at(instruction.reg);
			snprintf(instruction.regMnemonic, BUFFER_SIZE, "%s", reg);

			const char* rm = instruction.width ? registerWideTable.at(instruction.rm) : registerTable.at(instruction.rm);
			snprintf(instruction.rmMnemonic, BUFFER_SIZE, "%s", rm);

		} break;
		}
	}
}

/**
 * @brief Decode instruction using entry in 8086 table
 * @param instruction
 * @param entry
 */
void decodeInstruction(Instruction& instruction, InstructionEntry& entry, std::vector<uint8_t> program, int& programIndex)
{
	// Begin filling in instruction details
	instruction.opcode = entry.opcode;
	snprintf(instruction.mnemonic, BUFFER_SIZE, "%s", entry.mnemonic);

	// TODO: Do we need to adjust length or do anything new if dMask or wMask are present? 
	// Check if instruction contains Direction Mask
	if (entry.dMask != 0)
	{
		instruction.direction = program.at(programIndex) & entry.dMask;
	}

	// Check if instruction contains Direction Mask
	if (entry.wMask != 0)
	{
		instruction.width = program.at(programIndex) & entry.wMask;
	}

	switch (entry.size)
	{
	case 1:
	{
		decodeOneByteInstruction(instruction, entry, program, programIndex);
	} break;
	case 2:
	{
		decodeTwoByteInstruction(instruction, entry, program, programIndex);
	} break;
	}
}