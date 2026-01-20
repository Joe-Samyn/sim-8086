// sim8086.cpp : Defines the entry point for the application.
//

#include "sim8086.h"

#include <fstream>
#include <bitset>
#include <format>
#include <vector>

struct Instruction
{
	const char* description;
	uint8_t length;
	uint8_t opcode;
	uint8_t opcodeMask;
	uint8_t dMask;
	uint8_t wMask;
};


#define InstructionTable \
	X("Register/memory to/from register/memory", 2, 0b10001000, 0b11111100, 0b00000010, 0b00000001) \
	X("Immediate to register/memory", 2, 0b11000110, 0b11111110, 0, 0b00000001) \
	X("Immediate to register/memory", 2, 0b10110000, 0b11110000, 0, 0b00000001) \


#define X(desc, length, opcode, opcodeMask, dMask, wMask) { desc, length, opcode, opcodeMask, dMask, wMask },
std::vector<Instruction> instructionTable = {
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
	for (uint8_t byte : buffer)
	{
		// Get opcode 
		int instIndex = -1;
		for (int i = 0; i < instructionTable.size(); i++)
		{
			Instruction inst = instructionTable.at(i);
			uint8_t b = byte & inst.opcodeMask;
			if ((b | inst.opcode) == inst.opcode)
			{
				instIndex = i;
				break;
			}
		}

		if (instIndex != -1)
		{
			std::cout << std::format("Instruction: {}, {}", instructionTable.at(instIndex).description, std::bitset<8>(instructionTable.at(instIndex).opcode).to_string());
		}

	}


	return 0;
}
