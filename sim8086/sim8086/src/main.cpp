// sim8086.cpp : Defines the entry point for the application.
//

#include "sim8086.h"

#include <fstream>
#include <bitset>
#include <format>
#include <vector>
#include <unordered_map>


// TODO(joe): Trying to get MOD bits out of second byte. 

struct InstructionEntry
{
	const char* description;
	const char* mnemonic;
	uint8_t length;
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
	const char* mnemonic;
	uint8_t direction;
	uint8_t width;
	uint8_t registerA;
	const char* registerAName;
	uint8_t registerB;
	const char* registerBName;
};


#define InstructionTable \
	X("Register/memory to/from register/memory", "MOV", 2, 0b10001000, 0b11111100, 0b00000010, 0b00000001, 0b11000000, 0b00111000, 0b00000111) \
	X("Immediate to register/memory", "MOV", 2, 0b11000110, 0b11111110, 0, 0b00000001, 0b00000000, 0b00000000, 0b00000000) \


#define X(desc, mnemonic, length, opcode, opcodeMask, dMask, wMask, modMask, reg, rm) { desc, mnemonic, length, opcode, opcodeMask, dMask, wMask, modMask, reg, rm },
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




using namespace std;

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

	if (buffer.size() < 1)
	{
		return 1;
	}

	// Parse bytes
	std::cout << "\n\nbits 16\n\n\n";
	int currentByteIndex = 0;
	while (currentByteIndex < buffer.size())
	{
		int instIndex = -1;
		uint8_t currentByte = buffer.at(currentByteIndex);
		for (int i = 0; i < instructionTable.size(); i++)
		{
			// Get opcode from byte
			InstructionEntry inst = instructionTable.at(i);
			uint8_t b = buffer.at(currentByteIndex) & inst.opcodeMask;

			// Check if opcode is matches current instruction entry
			if (b == inst.opcode)
			{
				instIndex = i;
				break;
			}
		}

		// Check if an instruction was found
		if (instIndex == -1)
		{
			std::cout << std::format("The byte did not map to a valid 8086 instruction::{}\n", std::bitset<8>(currentByte).to_string());
			return 1;
		}

		InstructionEntry instructionEntry = instructionTable.at(instIndex);
		//std::cout << std::format("Instruction: {}, {}\n", instructionEntry.description, std::bitset<8>(instructionEntry.opcode).to_string());

		// Begin filling in instruction details
		Instruction instruction = { 0 };
		instruction.opcode = instructionEntry.opcode;
		instruction.mnemonic = instructionEntry.mnemonic;
		
		// TODO: Do we need to adjust length or do anything new if dMask or wMask are present? 
		// Check if instruction contains Direction Mask
		if (instructionEntry.dMask != 0)
		{
			instruction.direction = currentByte & instructionEntry.dMask;
		}

		// Check if instruction contains Direction Mask
		if (instructionEntry.wMask != 0)
		{
			instruction.width = currentByte & instructionEntry.wMask;
		}

		if (instructionEntry.modMask != 0)
		{
			currentByteIndex++;
			currentByte = buffer.at(currentByteIndex);
			uint8_t mod = currentByte & instructionEntry.modMask;

			switch (mod)
			{
				// Memory mode, no displacement
				case 0b00000000:
				{
					// TODO
				} break;

				// Memory mode, 8-bit displacement
				case 0b10000000:
				{
					// TODO
				} break;

				// Memory mode, 16-bit displacement
				case 0b01000000:
				{
					// TODO
				} break;

				// Register mode, no displacement
				case 0b11000000:
				{
					// Both operands are in registers, so decode the registers involved in the operation
					uint8_t registerA = (currentByte & instructionEntry.regMask) >> 3;
					uint8_t registerB = currentByte & instructionEntry.rmMask;

					
					instruction.registerA = instruction.direction ? registerA : registerB;
					instruction.registerB = instruction.direction? registerB: registerA;
					instruction.registerAName = instruction.width ? registerWideTable.at(instruction.registerA) : registerTable.at(instruction.registerA);
					instruction.registerBName = instruction.width ? registerWideTable.at(instruction.registerB) : registerTable.at(instruction.registerB);

				} break;
			}

			currentByteIndex++;
		}

		// Print instruction 
		std::cout << std::format("{} {}, {}\n", instruction.mnemonic, instruction.registerAName, instruction.registerBName);
	}


	return 0;
}
