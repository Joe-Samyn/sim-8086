#pragma once
#include "InstructionTable.h"
#include "Hardware.h"

#include <vector>
#include <optional>
#include <stdio.h>
#include <format>


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
uint16_t LoadWordData(struct CPU &cpu)
{
	uint8_t lowByte = cpu.memory[cpu.PC];
	uint16_t highByte = cpu.memory[++cpu.PC];
	uint16_t wideValue = (highByte << 8) | lowByte;
	return wideValue;
}

/**
 * @brief Read an 8-bit byte from `memory` at address `PC` and return it as a 16-bit unsigned integer.
 * @param cpu The CPU structure containing memory and PC.
 * @return The data loaded from memory as an unsigned 16-bit integer (upper bits zeroed).
 * @note This function relates to 8086 immediate byte operands, where 8-bit values are loaded from the instruction stream.
 */
int16_t LoadByteData(struct CPU &cpu)
{
	int8_t lowByte = static_cast<int8_t>(cpu.memory[cpu.PC]);
    int16_t data = static_cast<int16_t>(lowByte);
	return data;
}

/**
 * @brief Loads an immediate value from memory based on the specified width.
 * @param width 0 for 8-bit, 1 for 16-bit.
 * @param cpu The CPU structure containing memory and PC.
 * @return The loaded immediate value as a 16-bit unsigned integer.
 * @note Increments PC. In 8086, immediates follow opcodes and are used directly in instructions like MOV immediate.
 */
uint16_t LoadImmediate(uint8_t width, struct CPU &cpu)
{
	cpu.PC++;

	if (width) return LoadWordData(cpu);

	return LoadByteData(cpu);
}

/**
 * @brief Extracts the MOD field from the current opcode byte.
 * @param modMask Bitmask to isolate the MOD bits (typically 0xC0).
 * @param cpu The CPU structure.
 * @return The 2-bit MOD value (0-3), indicating addressing mode.
 * @note In 8086, MOD (bits 7-6 of MOD-RM byte) specifies memory/register mode: 00=mem no disp, 01=mem 8-bit disp, 10=mem 16-bit disp, 11=register.
 */
uint8_t GetMod(uint8_t modMask, struct CPU& cpu)
{
	return (cpu.memory[cpu.PC] & modMask) >> 6;
}

/**
 * @brief Extracts the RM field from the current opcode byte.
 * @param rmMask Bitmask to isolate the RM bits (typically 0x07).
 * @param cpu The CPU structure.
 * @return The 3-bit RM value (0-7), indicating register or effective address base.
 * @note In 8086, RM (bits 2-0 of MOD-RM byte) selects the register or base for addressing (e.g., BX+SI).
 */
uint8_t GetRm(uint8_t rmMask, struct CPU& cpu)
{
	return (cpu.memory[cpu.PC] & rmMask);
}

/**
 * @brief Extracts the REG field from the current opcode byte and formats the register mnemonic.
 * @param regMask Bitmask to isolate the REG bits.
 * @param regShift Number of bits to shift right to get the REG value.
 * @param instruction The instruction structure to store the mnemonic.
 * @param cpu The CPU structure.
 * @return The 3-bit REG value (0-7).
 * @note In 8086, REG (bits 5-3 of MOD-RM byte) specifies the source/destination register in reg-reg/mem operations.
 */
uint8_t GetReg(uint8_t regMask, uint8_t regShift, Instruction& instruction, struct CPU& cpu)
{
	return (cpu.memory[cpu.PC] & regMask) >> regShift;
}

/**
 * @brief Extracts a constant bit field from the current opcode byte.
 * @param constMask Bitmask to isolate the constant bits.
 * @param cpu The CPU structure.
 * @return The extracted bit value, shifted to the least significant position.
 * @note In 8086 arithmetic instructions, this extracts specific control bits from the opcode.
 */
uint8_t GetBitConst(uint8_t constMask, struct CPU& cpu)
{
	return (cpu.memory[cpu.PC] & constMask) >> 3;
}

/**
 * @brief Decodes the MOD-RM byte to determine the addressing mode and formats the operand mnemonic.
 * @param mod The MOD value (0-3).
 * @param rm The RM value (0-7).
 * @param instruction The instruction structure to update with the mnemonic.
 * @param cpu The CPU structure; PC is advanced as needed.
 * @note This function emulates 8086's microcode for parsing effective addresses, handling displacements and register modes.
 */
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
			instruction.rmMnemonic = std::format("[{}]", LoadWordData(cpu));
		}
		else
			instruction.rmMnemonic = std::format("[{}]", modEffectiveAddressTable.at(rm));
	} break;
	case 0x01:
	{
		// Need to sign extend properly to cast to signed int16, otherwise we will get incorrect values for negative displacements.
		int16_t data = static_cast<int16_t>(LoadImmediate(0, cpu));
		if (data < 0)
		{
			data = ~data + 1; // get the positive value of the negative displacement for printing
			instruction.rmMnemonic = std::format("[{} - {}]", modEffectiveAddressTable.at(rm), data);
		}
		else
			instruction.rmMnemonic = std::format("[{} + {}]", modEffectiveAddressTable.at(rm), data);
	} break;
	case 0x02:
	{
		int16_t data = static_cast<int16_t>(LoadImmediate(1, cpu));
		if (data < 0)
		{
			data = ~data + 1; // get the positive value of the negative displacement for printing
			instruction.rmMnemonic = std::format("[{} - {}]", modEffectiveAddressTable.at(rm), data);
		}
		else
			instruction.rmMnemonic = std::format("[{} + {}]", modEffectiveAddressTable.at(rm), data);
	} break;
	case 0x03:
	{
		instruction.rmMnemonic = std::format("{}", GetRegister(rm, instruction.width));
	} break;
	}
}

/**
 * @brief Decodes two-byte logic instructions (e.g., MOV reg/mem to reg/mem).
 * @param instruction The instruction structure to populate.
 * @param entry The instruction table entry with encoding details.
 * @param cpu The CPU structure; PC is advanced.
 * @note Handles 8086 instructions like MOV, ADD with MOD-RM and REG operands, extracting width, direction, and operands.
 */
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
		instruction.regMnemonic = std::format("{}", GetRegister(reg, instruction.width));
	}
	uint8_t mod = GetMod(logicEntry.modMask, cpu);
	uint8_t rm = GetRm(logicEntry.rmMask, cpu);
	DecodeMod(mod, rm, instruction, cpu);
}

/**
 * @brief Decodes two-byte logic instructions with immediate operands (e.g., MOV immediate to reg/mem).
 * @param instruction The instruction structure to populate.
 * @param entry The instruction table entry with encoding details.
 * @param cpu The CPU structure; PC is advanced.
 * @note Emulates 8086 microcode for instructions like MOV immediate, loading the immediate value after MOD-RM.
 */
void DecodeTwoByteLogicImmediate(Instruction& instruction, InstructionTableEntry& entry, struct CPU& cpu)
{
	struct TwoByteLogicImmediateEntry logicImmediateEntry = entry.encoding.twoByteLogicImmediateEntry;

	// Get width if w bit is present 
	if (logicImmediateEntry.wMask != 0) instruction.width = (cpu.memory[cpu.PC] & logicImmediateEntry.wMask);

	cpu.PC++;
	uint8_t mod = GetMod(logicImmediateEntry.modMask, cpu);
	uint8_t rm = GetRm(logicImmediateEntry.rmMask, cpu);
	DecodeMod(mod, rm, instruction, cpu);
	instruction.immediate = LoadImmediate(instruction.width, cpu);
}

/**
 * @brief Decodes one-byte logic instructions with immediate operands (e.g., MOV immediate to register).
 * @param instruction The instruction structure to populate.
 * @param entry The instruction table entry with encoding details.
 * @param cpu The CPU structure; PC is advanced.
 * @note Handles 8086 instructions like MOV AL/AX, immediate, where the register is encoded in the opcode byte.
 */
void DecodeOneByteLogicImmediate(Instruction& instruction, InstructionTableEntry& entry, struct CPU& cpu)
{
	struct OneByteLogicImmediateEntry logicImmediateEntry = entry.encoding.oneByteLogicImmediateEncoding;

	// Get width if w bit is present 
	instruction.width = (cpu.memory[cpu.PC] & logicImmediateEntry.wMask) >> logicImmediateEntry.wShift;

	uint8_t reg = (cpu.memory[cpu.PC] & logicImmediateEntry.regMask);
	instruction.regMnemonic = std::format("{}", GetRegister(reg, instruction.width));

	instruction.immediate = LoadImmediate(instruction.width, cpu);
}

/**
 * @brief Decodes accumulator instructions (e.g., MOV accumulator, memory/immediate).
 * @param instruction The instruction structure to populate.
 * @param entry The instruction table entry with encoding details.
 * @param cpu The CPU structure; PC is advanced.
 * @note Acts like microcode for 8086 accumulator operations, such as MOV AL/AX, [mem] or MOV AL/AX, immediate, where the accumulator is implied.
 */
void DecodeAccumulator(Instruction& instruction, InstructionTableEntry& entry, struct CPU& cpu)
{
	struct ThreeByteAccumulatorEntry accumulatorEntry = entry.encoding.threeByteAccumulatorEncoding;
	instruction.direction = accumulatorEntry.direction;
	instruction.width = (cpu.memory[cpu.PC] & accumulatorEntry.wMask);
    if (accumulatorEntry.hasAddress)
        instruction.address = LoadImmediate(1, cpu);
    else
        instruction.immediate = LoadImmediate(1, cpu);
    
	instruction.regMnemonic = std::format("{}", GetRegister(0x00, instruction.width));
    
}

void DecodeArithmeticTwoByteSigned(Instruction& instruction, InstructionTableEntry& entry, struct CPU& cpu)
{
	ArithmeticTwoByteImmedSignedEntry signedEntry = entry.encoding.arithmeticTwoByteImmedSignedEntry;

	uint8_t s = (cpu.memory[cpu.PC] & signedEntry.sMask) >> 1;
	instruction.width = cpu.memory[cpu.PC] & signedEntry.wMask;

	// Increment because all the following data is in byte 2
	cpu.PC++;
	uint8_t mod = GetMod(signedEntry.modMask, cpu);
	uint8_t rm = GetRm(signedEntry.rmMask, cpu);
	uint8_t bitConst = GetBitConst(signedEntry.constMask, cpu);
	DecodeMod(mod, rm, instruction, cpu);

	instruction.immediate = LoadImmediate(instruction.width, cpu);
}

/**
 * @brief Main decoding function that dispatches to specific decoders based on instruction category.
 * @param instruction The instruction structure to populate with decoded information.
 * @param entry The instruction table entry containing mnemonic and encoding details.
 * @param cpu The CPU structure; PC is advanced during decoding.
 * @note This function emulates the 8086's instruction decoding pipeline, routing opcodes to appropriate microcode-like handlers.
 */
void Decode(Instruction& instruction, InstructionTableEntry& entry, struct CPU& cpu)
{
	instruction.mnemonic = std::format("{}", entry.mnemonic);

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
	case ENCODING_ARITHMETIC_TWO_BYTE_IMMEDIATE_SIGNED:
	{
		DecodeArithmeticTwoByteSigned(instruction, entry, cpu);
	} break;
	}
}

/**
 * @brief Looks up an opcode byte in the instruction table.
 * @param byte The opcode byte to decode.
 * @return An optional containing the instruction table entry if found, or std::nullopt if invalid.
 * @note This function performs the initial opcode lookup in the 8086 decoding process, similar to the control store access.
 */
std::optional<InstructionTableEntry> decodeOpcode(const uint8_t byte)
{
	int instructionEntry = FindInstruction(byte);

	if (instructionEntry < 0) return std::nullopt;

	return std::make_optional(instructionTable.at(instructionEntry));
}
