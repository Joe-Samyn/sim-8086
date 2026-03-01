#pragma once
#include "InstructionTable.h"

#include <vector>
#include <optional>

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
uint16_t loadWordData(uint8_t memory[], uint16_t& PC)
{
	uint8_t lowByte = memory[PC];
	uint16_t highByte = memory[++PC];
	uint16_t wideValue = (highByte << 8) | lowByte;
	return wideValue;
}

/**
 * @brief Read one byte of data from emulator memory
 * @param program 
 * @param programIndex 
 * @return The data loaded from memory converted to an unsigned 8-bit integer
 */
uint8_t loadByteData(std::vector<uint8_t>& program, int& programIndex)
{
	programIndex++;
	return program.at(programIndex);
}

/**
 * @brief Decode the MOD data to determine value for RM field in assembly. 
 * The intel 8086 decoding syntax uses the MOD field to determine if the instruction is
 *	- Memory Mode w/ No Displcement:      0x00
 *  - Memory Mode w/ 8-bit Displacement:  0x01
 *  - Memory Mode w/ 16-bit Displacement: 0x02
 *  - Register mode w/ No Displacement:   0x03
 * @param instruction The instruction struct to add MOD and RM information too
 * @param entry The InstructionEntry that matches the opcode. This determines how an instruction is decoded. 
 * @param memory A reference to the emulator memory
 * @param PC A reference to the program counter
 * @return The number of bytes read from memory to decode the MOD and RM fields. This is used to update the program counter in the caller function.
 */
uint16_t getModRm(Instruction& instruction, InstructionEntry& entry, uint8_t memory[], uint16_t PC)
{
	uint16_t tempPC = PC + 1;
	uint8_t mod = (memory[tempPC] & entry.modMask) >> 6;
	uint8_t rm = (memory[tempPC] & entry.rmMask);

	switch (mod)
	{
	case 0x00:
	{	
		sprintf(instruction.rmMnemonic, "[%s]", modEffectiveAddressTable.at(rm));
	} break;
	case 0x01:
	{
		// decode
	} break;
	case 0x02:
	{
		// decode
	} break;
	case 0x03:
	{
		// decode
	} break;
	}

	return tempPC;
}

void getReg(Instruction& instruction, InstructionEntry& entry, uint8_t memory[], uint16_t PC)
{
	uint8_t reg = (memory[PC] & entry.regMask) >> entry.regShift;
	sprintf(instruction.regMnemonic, "%s", getRegister(reg, instruction.width));
}

/**
 * @brief Decode instruction using entry in 8086 table. This function acts as the ROM decoder in Intel 8086 hardware by 
 * determining the correct decode logic (microcode) for an instruction. 
 * @param instruction
 * @param entry
 * @returns The number of bytes read from memory to decode the instruction. This is used to update the program counter in the caller function.
 */
uint16_t decodeInstruction(Instruction& instruction, InstructionEntry& entry, uint8_t memory[], uint16_t PC)
{
	// Keep track of number of bytes read from memory to decode instruction
	uint16_t byteIncrement = 0;

	// Set mnemonic 
	sprintf(instruction.mnemonic, "%s", entry.mnemonic);

	// Get width if w bit is present 
	if (entry.wMask != 0)  instruction.width = (memory[PC] & entry.wMask) >> entry.wShift;

	// Get direction if d bit is present, direction is always in bit 1 if it is present so we can just shift right by 1 to get the value
	if (entry.dMask != 0) instruction.direction = (memory[PC] & entry.dMask) >> 1;
	
	// Get MOD byte and RM field if necessary
	if (entry.hasModByte)
		byteIncrement = getModRm(instruction, entry, memory, PC);

	// Get reg field if necessary
	if (entry.hasModByte && entry.hasReg)
		getReg(instruction, entry, memory, PC + 1);


	return byteIncrement;
}

std::optional<InstructionEntry> decodeOpcode(const uint8_t byte)
{
	int instructionEntry = findInstruction(byte);

	if (instructionEntry < 0) return std::nullopt;

	return std::make_optional(instructionTable.at(instructionEntry));
}