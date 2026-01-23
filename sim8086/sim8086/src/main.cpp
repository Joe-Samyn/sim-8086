// sim8086.cpp : Defines the entry point for the application.
//

#include "sim8086.h"

#include <fstream>
#include <bitset>
#include <format>
#include <vector>


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
};

struct Instruction
{
	uint8_t opcode;
	const char* mnemonic;
	uint8_t direction;
	uint8_t width;
};


#define InstructionTable \
	X("Register/memory to/from register/memory", "MOV", 2, 0b10001000, 0b11111100, 0b00000010, 0b00000001) \
	X("Immediate to register/memory", "MOV", 2, 0b11000110, 0b11111110, 0, 0b00000001) \
	X("Immediate to register/memory", "MOV", 2, 0b10110000, 0b11110000, 0, 0b00000001) \


#define X(desc, length, opcode, opcodeMask, dMask, wMask) { desc, length, opcode, opcodeMask, dMask, wMask },
std::vector<InstructionEntry> instructionTable = {
	InstructionTable
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
	for (uint8_t byte : buffer)
	{
		int instIndex = -1;
		for (int i = 0; i < instructionTable.size(); i++)
		{
			// Get opcode from byte
			InstructionEntry inst = instructionTable.at(i);
			uint8_t b = byte & inst.opcodeMask;

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
			std::cout << std::format("The byte did not map to a valid 8086 instruction::{}\n", std::bitset<8>(byte).to_string());
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
			instruction.direction = byte & instructionEntry.dMask;
		}

		// Check if instruction contains Direction Mask
		if (instructionEntry.wMask != 0)
		{
			instruction.width = byte & instructionEntry.wMask;
		}

		// check instruction length, if longer than 1, get second byte
		// Need to determine what data to get from subsequent bytes

		// Print instruction 
		std::cout << std::format("{} \n", instruction.mnemonic);
	}


	return 0;
}
