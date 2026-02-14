#pragma once
#include <vector>
#include <cstdint>
#include <unordered_map>

const int BUFFER_SIZE = 32;

/**
 * @brief Represents a decoded instructions
 */
struct Instruction
{
	uint8_t opcode;
	char mnemonic[BUFFER_SIZE];
	uint8_t direction;
	uint8_t width;
	uint8_t reg;
	// The human readable name for the register (i.e. AX, BX, etc.)
	char regMnemonic[BUFFER_SIZE];
	uint8_t rm;
	// The human redable name for the register stored in R/M when R/M is used to hold register info 
	char rmMnemonic[BUFFER_SIZE];
	int16_t immediate;
};


/* ====================================================================== */
/* ==============     Intel 8086 Instruction Table    =================== */
/* ====================================================================== */

/**
 * @brief Represents an instruction entry in the Intel 8086 Instruction table
 */
struct InstructionEntry
{
	const char* description;
	const char* mnemonic;
	uint8_t size;
	uint8_t opcode;
	uint8_t opcodeMask;
	uint8_t dMask;
	uint8_t wMask;
	uint8_t modMask;
	uint8_t regMask;
	uint8_t rmMask;
};

#define InstructionTable \
	X("Register/memory to/from register/memory", "MOV", 2, 0b10001000, 0b11111100, 0b00000010, 0b00000001, 0b11000000, 0b00111000, 0b00000111) \
	X("Immediate to register/memory", "MOV", 1, 0b10110000, 0b11110000, 0, 0b00001000, 0b00000000, 0b00000111, 0b00000000) \


#define X(desc, mnemonic, size, opcode, opcodeMask, dMask, wMask, modMask, reg, rm) { desc, mnemonic, size, opcode, opcodeMask, dMask, wMask, modMask, reg, rm },
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


/* ====================================================================== */
/* ============== Effective Address Calculation Table =================== */
/* ====================================================================== */

/**
 * @brief Represents an effective address calculation found in the Intel 8086 Manual MOD section
 */
struct EffectiveAddrCalculation
{
	uint8_t registerA;
	uint8_t registerB;
	uint8_t displacement;
	const char* calcLiteral;
};

#define ModCalculations \
	X(/* R/M = 000*/ 0b00000000, /* BX */ 0b00000011, /* SI*/ 0b00000101, /* 0 */ 0b0, "BX + SI") \
	X(/* R/M = 001*/ 0b00000001, /* BX */ 0b00000011, /* DI*/ 0b00000111, /* 0 */ 0b0, "BX + DI") \
	X(/* R/M = 010*/ 0b00000010, /* BP */ 0b00000101, /* SI*/ 0b00000101, /* 0 */ 0b0, "BP + SI") \
	X(/* R/M = 011*/ 0b00000011, /* BP */ 0b00000101, /* DI*/ 0b00000111, /* 0 */ 0b0, "BP + DI") \
	X(/* R/M = 100*/ 0b00000100, /* SI */ 0b00000101, /* 0 */ 0b0,        /* 0 */ 0b0, "SI") \
	X(/* R/M = 101*/ 0b00000101, /* DI */ 0b00000111, /* 0 */ 0b0,        /* 0 */ 0b0, "DI") \
	X(/* R/M = 110*/ 0b00000110, /* DA */ 0b0,        /* SI*/ 0b0,        /* 0 */ 0b0, "[]") \
	X(/* R/M = 111*/ 0b00000111, /* BX */ 0b00000011, /* 0 S*/ 0b0,       /* 0 */ 0b0, "BX") \

#define X(code, regA, regB, displacement, calcLiteral) { code, {regA, regB, displacement, calcLiteral} },
std::unordered_map<uint8_t, EffectiveAddrCalculation> modEffectiveAddressTable = {
	ModCalculations
};
#undef X


/* ====================================================================== */
/* ==============      Table operation functions      =================== */
/* ====================================================================== */


// TODO: This might be better off returning an instruction entry instead of an index
/**
 * @brief Find the instruction whose opcode matches the byte
 * @param byte
 * @return
 */
int findInstruction(uint8_t byte)
{
	for (int i = 0; i < instructionTable.size(); i++)
	{
		// Get opcode from byte
		InstructionEntry inst = instructionTable.at(i);
		uint8_t b = byte & inst.opcodeMask;

		// If byte matches opcode, break out and return index of instruction
		if (b == inst.opcode)
		{
			return i;
		}
	}

	// Byte doesn't match any instructions
	return -1;
}