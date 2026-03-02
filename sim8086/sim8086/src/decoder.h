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
uint16_t loadWordData(uint8_t memory[], uint16_t &PC)
{
	uint8_t lowByte = memory[PC];
	uint16_t highByte = memory[++PC];
	uint16_t wideValue = (highByte << 8) | lowByte;
	return wideValue;
}

/**
 * @brief Read an 8-bit byte from `memory` at address `PC + 1` and return it as a 16-bit unsigned integer.
 * @param program 
 * @param programIndex 
 * @return The data loaded from memory converted to an unsigned 8-bit integer
 */
uint16_t loadByteData(uint8_t memory[], uint16_t &PC)
{
	return static_cast<uint16_t>(memory[++PC]);
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
 */
void getModRm(Instruction& instruction, InstructionEntry& entry, uint8_t memory[], uint16_t& PC)
{
	uint8_t mod = (memory[PC] & entry.modMask) >> 6;
	uint8_t rm = (memory[PC] & entry.rmMask);

	switch (mod)
	{
	// Memory mode, no displacement
	case 0x00:
	{	
		// Special case of MOD that indicates direct memory access with no displacement. 
		if (rm == 0x6)
			sprintf(instruction.rmMnemonic, "[%d]", loadWordData(memory, ++PC));
		else
			sprintf(instruction.rmMnemonic, "[%s]", modEffectiveAddressTable.at(rm));
	} break;
	// Memory mode, 8-bit displacement
	case 0x01:
	{
		int16_t data = static_cast<int16_t>(loadByteData(memory, PC));
		sprintf(instruction.rmMnemonic, "[%s + %d]", modEffectiveAddressTable.at(rm), data);
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
}

void getReg(Instruction& instruction, InstructionEntry& entry, uint8_t memory[], uint16_t& PC)
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
void decodeInstruction(Instruction& instruction, InstructionEntry& entry, uint8_t memory[], uint16_t &PC)
{

	// Set mnemonic 
	sprintf(instruction.mnemonic, "%s", entry.mnemonic);

	// Get width if w bit is present 
	if (entry.wMask != 0)  instruction.width = (memory[PC] & entry.wMask) >> entry.wShift;

	// Get direction if d bit is present, direction is always in bit 1 if it is present so we can just shift right by 1 to get the value
	if (entry.dMask != 0) instruction.direction = (memory[PC] & entry.dMask) >> 1;

	// Get reg field if necessary
	if (entry.isRegInOpcode) getReg(instruction, entry, memory, PC);

	// Get MOD byte, RM field and REG field
	if (entry.hasModByte)
	{
		PC++;
		getReg(instruction, entry, memory, PC);
		getModRm(instruction, entry, memory, PC);
	}

}

std::optional<InstructionEntry> decodeOpcode(const uint8_t byte)
{
	int instructionEntry = findInstruction(byte);

	if (instructionEntry < 0) return std::nullopt;

	return std::make_optional(instructionTable.at(instructionEntry));
}