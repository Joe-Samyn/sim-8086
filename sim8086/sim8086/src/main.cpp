// sim8086.cpp : Defines the entry point for the application.

#include "sim8086.h"

#include <fstream>
#include <bitset>
#include <format>
#include <vector>
#include <unordered_map>


void printInstruction(Instruction &inst)
{
	if (inst.immediate)
	{
		std::cout << std::format("{} {}, {}\n", inst.mnemonic, inst.regMnemonic, inst.immediate);
	}
	else
	{
		if (inst.direction)
		{
			std::cout << std::format("{} {}, {}\n", inst.mnemonic, inst.regMnemonic, inst.rmMnemonic);
		}
		else
		{
			std::cout << std::format("{} {}, {}\n", inst.mnemonic, inst.rmMnemonic, inst.regMnemonic);
		}
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
