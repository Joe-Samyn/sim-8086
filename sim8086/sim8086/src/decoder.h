#pragma once
#include "InstructionTable.h"
#include "Hardware.h"

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

void loadAddress(Instruction& instruction, InstructionEntry& entry, struct CPU& cpu)
{
	cpu.PC++;
	instruction.address = loadWordData(cpu);
}

void loadImmediate(Instruction& instruction, InstructionEntry& entry, struct CPU &cpu)
{
	cpu.PC++;

	// Instruction uses W bit to determine if immediate is byte or word, and the width of the instruction is 1 (i.e. word), then load a word
	if (entry.immUsesW && instruction.width == 1)
		instruction.immediate = loadWordData(cpu);
	// Immediate does not use W bit, but has immediate value, so load word by default
	else if (!entry.immUsesW && entry.hasImmediate)
		instruction.immediate = loadWordData(cpu);
	// Immediate uses W bit to determine if immediate is byte or word, and the width of the instruction is 0 (i.e. byte), then load a byte
	else
		instruction.immediate = loadByteData(cpu);
}

void getReg(Instruction& instruction, InstructionEntry& entry, struct CPU &cpu)
{
	uint8_t reg = (cpu.memory[cpu.PC] & entry.regMask) >> entry.regShift;
	sprintf(instruction.regMnemonic, "%s", getRegister(reg, instruction.width));
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
void getModRm(Instruction& instruction, InstructionEntry& entry, struct CPU &cpu)
{
	uint8_t mod = (cpu.memory[cpu.PC] & entry.modMask) >> 6;
	uint8_t rm = (cpu.memory[cpu.PC] & entry.rmMask);

	switch (mod)
	{
	// Memory mode, no displacement
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
	// Memory mode, 8-bit displacement
	case 0x01:
	{
		cpu.PC++;
		int16_t data = static_cast<int16_t>(loadByteData(cpu));
		sprintf(instruction.rmMnemonic, "[%s + %d]", modEffectiveAddressTable.at(rm), data);
	} break;
	// Memory mode, 16-bit displacement
	case 0x02:
	{
		cpu.PC++;
		int16_t data = static_cast<int16_t>(loadWordData(cpu));
		sprintf(instruction.rmMnemonic, "[%s + %d]", modEffectiveAddressTable.at(rm), data);
	} break;
	// Register mode, no displacement
	case 0x03:
	{
		sprintf(instruction.rmMnemonic, "%s", getRegister(rm, instruction.width));
	} break;
	}
}

/**
 * @brief Decode instruction using entry in 8086 table. This function acts as the ROM decoder in Intel 8086 hardware by 
 * determining the correct decode logic (microcode) for an instruction. 
 * @param instruction
 * @param entry
 * @returns The number of bytes read from memory to decode the instruction. This is used to update the program counter in the caller function.
 */
void decodeInstruction(Instruction& instruction, InstructionEntry& entry, struct CPU &cpu)
{
	// Set mnemonic 
	sprintf(instruction.mnemonic, "%s", entry.mnemonic);

	// Get width if w bit is present 
	if (entry.wMask != 0)  instruction.width = (cpu.memory[cpu.PC] & entry.wMask) >> entry.wShift;

	// Get direction if d bit is present, direction is always in bit 1 if it is present so we can just shift right by 1 to get the value
	if (entry.dMask != 0) instruction.direction = (cpu.memory[cpu.PC] & entry.dMask) >> 1;

	// Get reg field if necessary
	if (entry.isRegInOpcode) getReg(instruction, entry, cpu);

	// Get MOD byte, RM field and REG field
	if (entry.hasModByte)
	{
		cpu.PC++;
		if (entry.hasReg) getReg(instruction, entry, cpu);
		getModRm(instruction, entry, cpu);
	}

	// Load immediate value
	if (entry.hasImmediate) loadImmediate(instruction, entry, cpu);
	
	// Load address 
	if (entry.hasAddress) loadAddress(instruction, entry, cpu);

	// If no reg or mod byte is specified, it most likely means the instruction is using the accumulator register, so we can just set the reg mnemonic to AX or AL depending on the width of the instruction
	if (!entry.hasModByte && !entry.hasReg)
	{
		instruction.direction = 1; // Set direction to 1 so that reg is always first in assembly output
		if (instruction.width == 1)
			sprintf(instruction.regMnemonic, "%s", "AX");
		else
			sprintf(instruction.regMnemonic, "%s", "AL");
	}

}

std::optional<InstructionEntry> decodeOpcode(const uint8_t byte)
{
	int instructionEntry = findInstruction(byte);

	if (instructionEntry < 0) return std::nullopt;

	return std::make_optional(instructionTable.at(instructionEntry));
}