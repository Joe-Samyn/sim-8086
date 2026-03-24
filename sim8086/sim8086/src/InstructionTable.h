#pragma once
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <string>

const int BUFFER_SIZE = 32;

enum EncodingCategory {
	ENCODING_TWO_BYTE_LOGIC,
	ENCODING_THREE_BYTE_ACCUMULATOR,
	ENCODING_TWO_BYTE_LOGIC_IMMEDIATE,
	ENCODING_ONE_BYTE_LOGIC_IMMEDIATE,
	ENCODING_ARITHMETIC_TWO_BYTE_IMMEDIATE_SIGNED
};

/**
 * @brief Represents a decoded instructions
 * TODO: This can probably go away at some point and we can just write directly to a buffer or file. Its more for debugging
 */
struct Instruction
{
	uint8_t direction;				// Determines if reg or rm goes first in assembly output
	uint8_t width;					// Width of the register or data 
	uint8_t sign;
	int16_t immediate;				// Immediate value if there is one
	uint16_t address;				// Address if instruction has an address (i.e. MOV AL, [1234h])
	std::string mnemonic;		// Human readable assembly language mnemonic for the instruction
	std::string regMnemonic;	// The human readable name for the register (i.e. AX, BX, etc.)
	std::string rmMnemonic;	// The human redable name for the register stored in R/M when R/M is used to hold register info 
};


/* ====================================================================== */
/* ==============     Intel 8086 Instruction Table    =================== */
/* ====================================================================== */

struct TwoByteLogicEntry
{
	uint8_t dMask;
	uint8_t wMask;
	uint8_t modMask;
	uint8_t regMask;
	uint8_t regShift;
	uint8_t rmMask;
};

struct TwoByteLogicImmediateEntry
{
	uint8_t wMask;
	uint8_t modMask;
	uint8_t rmMask;
};

struct ThreeByteAccumulatorEntry
{
	uint8_t direction;
	uint8_t wMask;
	bool hasAddress;
};

struct OneByteLogicImmediateEntry
{
	uint8_t wMask;
	uint8_t wShift;
	uint8_t regMask;
};

struct ArithmeticTwoByteImmedSignedEntry 
{
	uint8_t sMask;
	uint8_t wMask;
	uint8_t modMask;
	uint8_t rmMask;
	uint8_t constMask;
	uint8_t direction;
};

struct InstructionTableEntry
{
	uint8_t opcode;
	uint8_t opcodeMask;
	const char* mnemonic;
	EncodingCategory category;
	union
	{
		struct TwoByteLogicEntry twoByteLogicEntry;
		struct TwoByteLogicImmediateEntry twoByteLogicImmediateEntry;
		struct ThreeByteAccumulatorEntry threeByteAccumulatorEncoding;
		struct OneByteLogicImmediateEntry oneByteLogicImmediateEncoding;
		struct ArithmeticTwoByteImmedSignedEntry arithmeticTwoByteImmedSignedEntry;

	} encoding;
};

// TODO (joe): Maybe remove strings from this table and create a mapping table of opcode -> mnemonic
#define InstructionTableEntries \
	X(0x88, 0xFC, "MOV", ENCODING_TWO_BYTE_LOGIC, 0x02,  0x01, 0xC0, 0x38, 0x3, 0x07), \
	X(0xC6, 0xFE, "MOV", ENCODING_TWO_BYTE_LOGIC_IMMEDIATE, 0x01, 0xC0, 0x07), \
	X(0xB0, 0xF0, "MOV", ENCODING_ONE_BYTE_LOGIC_IMMEDIATE, 0x08, 0x03, 0x07), \
	X(0xA0, 0xFE, "MOV", ENCODING_THREE_BYTE_ACCUMULATOR, 0x01, 0x01, true), \
	X(0xA2, 0xFE, "MOV", ENCODING_THREE_BYTE_ACCUMULATOR, 0x00, 0x01, true), \
	X(0x00, 0xFC, "ADD", ENCODING_TWO_BYTE_LOGIC, 0x02,  0x01, 0xC0, 0x38, 0x3, 0x07), \
	X(0x80, 0xFC, "ADD", ENCODING_ARITHMETIC_TWO_BYTE_IMMEDIATE_SIGNED,  0x02, 0x01, 0xC0, 0x07, 0x38, 0x01), \
    X(0x04, 0xFE, "ADD", ENCODING_THREE_BYTE_ACCUMULATOR, 0x01, 0x01, false), \
	X(0x14, 0xFE, "ADC", ENCODING_THREE_BYTE_ACCUMULATOR, 0x01, 0x01, false), \
	X(0x10, 0xFC, "ADC", ENCODING_TWO_BYTE_LOGIC, 0x02,  0x01, 0xC0, 0x38, 0x3, 0x07) \

#define X(opcode, opcodeMask, mnemonic, category, ...) { opcode, opcodeMask, mnemonic, category, { __VA_ARGS__ } }
std::vector<InstructionTableEntry> instructionTable = {
	InstructionTableEntries
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
const char* GetRegister(uint8_t reg, uint8_t width)
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
int FindInstruction(uint8_t byte)
{
	for (int i = 0; i < instructionTable.size(); i++)
	{
		// Get opcode from byte
		InstructionTableEntry inst = instructionTable.at(i);
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
