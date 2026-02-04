// sim8086.cpp : Defines the entry point for the application.
//

/*
TODO: 
	- We could just hardcode the buffer to specific size that matches the program memory size
	of most 8086 systems/chips since we are building a simulator
*/

#include "sim8086.h"

#include <fstream>
#include <bitset>
#include <format>
#include <vector>
#include <unordered_map>

const int BUFFER_SIZE = 32;


// TODO(joe): Trying to get MOD bits out of second byte. 

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
};

struct EffectiveAddrCalculation
{
	uint8_t registerA;
	uint8_t registerB;
	uint8_t displacement;
	const char* calcLiteral;
};


#define InstructionTable \
	X("Register/memory to/from register/memory", "MOV", 2, 0b10001000, 0b11111100, 0b00000010, 0b00000001, 0b11000000, 0b00111000, 0b00000111) \
	X("Immediate to register/memory", "MOV", 2, 0b11000110, 0b11111110, 0, 0b00000001, 0b00000000, 0b00000000, 0b00000000) \


#define X(desc, mnemonic, size, opcode, opcodeMask, dMask, wMask, modMask, reg, rm) { desc, mnemonic, size, opcode, opcodeMask, dMask, wMask, modMask, reg, rm },
std::vector<InstructionEntry> instructionTable = {
	InstructionTable
};
#undef X

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




using namespace std;

void printInstruction(Instruction &inst)
{
	// Destination is in reg field
	if (inst.direction)
	{
		std::cout << std::format("{} {}, {}\n", inst.mnemonic, inst.regMnemonic, inst.rmMnemonic);
	}
	else
	{
		std::cout << std::format("{} {}, {}\n", inst.mnemonic, inst.rmMnemonic, inst.regMnemonic);
	}
}

/**
 * @brief Reads binary file into memory
 * @param filePath 
 * @param buffer 
 */
void readBinaryFile(char* filePath, std::vector<uint8_t> &buffer)
{
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);

	if (!file.is_open() && errno == ENOENT)
	{
		std::cerr << "File does not exist.\n";
		return;
	}

	// Determine file size 
	std::streamsize file_size = file.tellg();
	std::cout << "File size: " << file_size << "B" << std::endl;

	// Seek back to beginning of file 
	file.seekg(0, std::ios::beg);
	buffer.resize(file_size);

	file.read(reinterpret_cast<char*>(buffer.data()), file_size);

	std::cout << "File read successfully.\n";
}

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
			EffectiveAddrCalculation calc = modEffectiveAddressTable.at(instruction.rm);

			// NOTE: When decoding for running a program, we would perform calculation here? 
			instruction.reg = (entry.regMask & program.at(programIndex)) >> 3;
			const char* reg = instruction.width ? registerWideTable.at(instruction.reg) : registerTable.at(instruction.reg);
			snprintf(instruction.regMnemonic, BUFFER_SIZE, "%s", reg);

			// Get register info
			snprintf(instruction.rmMnemonic, BUFFER_SIZE, "[%s]", calc.calcLiteral);

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
			uint8_t displacement = program.at(programIndex);

			// Get register info
			snprintf(instruction.rmMnemonic, BUFFER_SIZE, "[%s + %d]", calc.calcLiteral, displacement);
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
			programIndex++;
			uint8_t displacementLow = program.at(programIndex);
			programIndex++;
			uint16_t displacementHigh = program.at(programIndex);
			uint16_t displacement = (displacementHigh << 8) | displacementLow;

			// Get register info
			snprintf(instruction.rmMnemonic, BUFFER_SIZE, "[%s + %d]", calc.calcLiteral, displacement);
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
void decodeInstruction(Instruction &instruction, InstructionEntry &entry, std::vector<uint8_t> program, int &programIndex)
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
		// TODO: Implement 1 byte instruction parsing
	} break;
	case 2:
	{
		decodeTwoByteInstruction(instruction, entry, program, programIndex);
	} break;
	}
}


int main(int argc, char* argv[])
{
	std::vector<uint8_t> buffer;

	// No filepath is present 
	if (argc < 2)
	{
		std::cout << "No input file[s] found." << std::endl;
		return 1;
	}

	std::cout << "Decoding instructions in binary: " << argv[1] << std::endl;

	// Read binary file into a vector if bytes
	readBinaryFile(argv[1], buffer);

	// Ensure file was read into buffer
	if (buffer.size() < 1)
	{
		return 1;
	}

	// Begin decoding bytes one at a time
	std::cout << "\n\nbits 16\n\n\n";
	int programIndex = 0;
	while (programIndex < buffer.size())
	{
		
		uint8_t currentByte = buffer.at(programIndex);
		int instructionIndex = findInstruction(currentByte);

		// Check if an instruction was found
		if (instructionIndex == -1)
		{
			std::cout << std::format("The byte did not map to a valid 8086 instruction::{}\n", std::bitset<8>(currentByte).to_string());
			return 1;
		}

		InstructionEntry instructionEntry = instructionTable.at(instructionIndex);
		//std::cout << std::format("Instruction: {}, {}\n", instructionEntry.description, std::bitset<8>(instructionEntry.opcode).to_string());

		// Begin filling in instruction details
		Instruction instruction = { 0 };
		decodeInstruction(instruction, instructionEntry, buffer, programIndex);

		// Print instruction 
		printInstruction(instruction);

		programIndex++;
	}


	return 0;
}
