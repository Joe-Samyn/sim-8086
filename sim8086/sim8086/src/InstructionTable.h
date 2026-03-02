#pragma once
#include <vector>
#include <cstdint>
#include <unordered_map>

const int BUFFER_SIZE = 32;

enum InstructionClassification {
	ONE_BYTE_LOGIC,
	ONE_BYTE_DATA,
	TWO_BYTE_LOGIC,
};

/**
 * @brief Represents a decoded instructions
 * TODO: This can probably go away at some point and we can just write directly to a buffer or file. Its more for debugging
 */
struct Instruction
{
	uint8_t direction;				// Determines if reg or rm goes first in assembly output
	uint8_t width;					// Width of the register or data 
	int16_t immediate;				// Immediate value if there is one
	char mnemonic[BUFFER_SIZE];		// Human readable assembly language mnemonic for the instruction
	char regMnemonic[BUFFER_SIZE];	// The human readable name for the register (i.e. AX, BX, etc.)
	char rmMnemonic[BUFFER_SIZE];	// The human redable name for the register stored in R/M when R/M is used to hold register info 
};


/* ====================================================================== */
/* ==============     Intel 8086 Instruction Table    =================== */
/* ====================================================================== */

/**
 * @brief Represents an instruction entry in the Intel 8086 Instruction table
 */
struct InstructionEntry
{
	// Opcode and mnemonic for the instruction
	uint8_t opcode;
	const char* mnemonic;

	// direction - determines if reg is src or dest
	uint8_t dMask;

	// width - width of width of register and data
	bool immUsesW;
	uint8_t wMask;
	uint8_t wShift;

	// mod byte extraction 
	bool hasModByte;
	uint8_t modMask;

	// reg extraction
	bool hasReg;
	bool isRegInOpcode;
	uint8_t regMask;
	uint8_t regShift;

	// r/m extraction
	uint8_t rmMask;
};

// TODO (joe): Maybe remove strings from this table and create a mapping table of opcode -> mnemonic
#define InstructionTable \
	X(0x88, "MOV", 0x2, false, 0x1, 0x0, true, 0xC0, true, false, 0x38, 0x3, 0x7) \

#define X(opcode, mnemonic, dMask, immUsesW, wMask, wShift, hasModByte, modMask, hasReg, isRegInOpcode, regMask, regShift, rmMask) { opcode, mnemonic, dMask, immUsesW, wMask, wShift, hasModByte, modMask, hasReg, isRegInOpcode, regMask, regShift, rmMask },
std::vector<InstructionEntry> instructionTable = {
	InstructionTable
};
#undef X

/* ====================================================================== */
/* ==============     Register (1 & 2 byte) Table     =================== */
/* ====================================================================== */

#define RegistersWide8086 \
	X(0b00000000, "AX") \
	X(0b00000001, "CX") \
	X(0b00000010, "DX") \
	X(0b00000011, "BX") \
	X(0b00000100, "SP") \
	X(0b00000101, "BP") \
	X(0b00000110, "SI") \
	X(0b00000111, "DI") \

#define Registers8086 \
	X(0b00000000, "AL") \
	X(0b00000001, "CL") \
	X(0b00000010, "DL") \
	X(0b00000011, "BL") \
	X(0b00000100, "AH") \
	X(0b00000101, "CH") \
	X(0b00000110, "DH") \
	X(0b00000111, "BH") \

#define X(name, code) {name, code},
std::unordered_map<uint8_t, const char*> registerWideTable = {
	RegistersWide8086
};

std::unordered_map<uint8_t, const char*> registerTable = {
	Registers8086
};
#undef X

/**
 * @brief Get register mnemonic from register code and width. This is used to decode the reg field in the mod byte when it is used to hold register information.
 * @param reg 
 * @param width 
 * @return register mnemonic
 */
const char* getRegister(uint8_t reg, uint8_t width)
{
	// Get 16 bit register
	if (width)
		return registerWideTable.at(reg);

	// Get 8 bit register
	return registerTable.at(reg);
}


/* ====================================================================== */
/* ============== Effective Address Calculation Table =================== */

#define ModCalculations \
	X(/* R/M = 000*/ 0b00000000, "BX + SI") \
	X(/* R/M = 001*/ 0b00000001, "BX + DI") \
	X(/* R/M = 010*/ 0b00000010, "BP + SI") \
	X(/* R/M = 011*/ 0b00000011, "BP + DI") \
	X(/* R/M = 100*/ 0b00000100, "SI") \
	X(/* R/M = 101*/ 0b00000101, "DI") \
	X(/* R/M = 110*/ 0b00000110, "[]") \
	X(/* R/M = 111*/ 0b00000111, "BX") \

#define X(code, calcLiteral) { code, calcLiteral },
std::unordered_map<uint8_t, const char*> modEffectiveAddressTable = {
	ModCalculations
};
#undef X


/* ====================================================================== */
/* ==============      Table operation functions      =================== */
/* ====================================================================== */


// TODO: This might be better off returning an instruction entry instead of an index
/**
 * @brief Search instruction table for instruction that matches the opcode in the byte. 
 * @param byte The byte containing the opcode to search for in the instruction table.
 * @return Returns index of instruction in instruction table if found, otherwise returns -1.
 */
int findInstruction(uint8_t byte)
{
	for (int i = 0; i < instructionTable.size(); i++)
	{
		// Get opcode from byte
		InstructionEntry inst = instructionTable.at(i);
		uint8_t b = byte & inst.opcode;

		// If byte matches opcode, break out and return index of instruction
		if (b == inst.opcode)
		{
			return i;
		}
	}

	// Byte doesn't match any instructions
	return -1;
}