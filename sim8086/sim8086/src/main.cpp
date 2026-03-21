// sim8086.cpp : Defines the entry point for the application.

#include "sim8086.h"

#include <fstream>
#include <bitset>
#include <format>
#include <vector>
#include <unordered_map>
#include <string>

struct CPU cpu = { 0 };

void displayCpuState()
{
    std::cout << std::format("PC: {}\nCurrent Byte: {}\n", cpu.PC, std::bitset<8>(cpu.memory[cpu.PC]).to_string());
}

// TODO: We can get more granular on which instructions need to have size defined (word, byte) instead of doing all of them
std::string formatInstruction(Instruction &inst)
{
	if (inst.immediate)
	{
		if (inst.rmMnemonic[0] != '\0')
            if (inst.width)
                return std::format("{} word {}, {}\n", inst.mnemonic, inst.rmMnemonic, inst.immediate);
            else
                return std::format("{} byte {}, {}\n", inst.mnemonic, inst.rmMnemonic, inst.immediate);
		else
            if (inst.width)
                return std::format("{} word {}, {}\n", inst.mnemonic, inst.regMnemonic, inst.immediate);
            else
                return std::format("{} byte {}, {}\n", inst.mnemonic, inst.regMnemonic, inst.immediate);
	}
    else if (inst.address)
    {
        if (inst.direction)
            return std::format("{} {}, [{}]\n", inst.mnemonic, inst.regMnemonic, inst.address);

        return std::format("{} [{}], {}\n", inst.mnemonic, inst.address, inst.regMnemonic);
    }
	else
    {
        if (inst.direction)
        {
            if (inst.width)
                return std::format("{} word {}, {}\n", inst.mnemonic, inst.regMnemonic, inst.rmMnemonic);
            else
                return std::format("{} byte {}, {}\n", inst.mnemonic, inst.regMnemonic, inst.rmMnemonic);
        }
        else
        {
            if (inst.width)
                return std::format("{} word {}, {}\n", inst.mnemonic, inst.rmMnemonic, inst.regMnemonic);
            else
                return std::format("{} byte {}, {}\n", inst.mnemonic, inst.rmMnemonic, inst.regMnemonic);
        }
    }
}

/**
 * @brief Reads binary file into memory
 * @param filePath
 * @param buffer
 *
 * TODO: Really should be part of initializing the machine
 */
void readBinaryFile(char* filePath)
{
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);

	if (!file.is_open() && errno == ENOENT)
	{
		std::cerr << "File does not exist.\n";
		return;
	}

	// Determine file size
	std::streamsize file_size = file.tellg();
	file.seekg(0, std::ios::beg);

	file.read(reinterpret_cast<char*>(cpu.memory), file_size);
}

// TODO: In the this decode loop, rather than passing PC as a reference, we should probably just return the number of bytes read and have the caller update PC.
// This would make it easier to handle instructions that have variable length (i.e. some instructions have an optional mod byte, some have an optional immediate value, etc.)
std::vector<Instruction> beginDecode()
{
	std::vector<Instruction> decodedInstructions = {};
	while (cpu.memory[cpu.PC] != 0)
	{
		uint8_t currentByte = cpu.memory[cpu.PC];

		// Get opcode from byte
		// TODO: This could be cleaned up by having a global error function
		std::optional<InstructionTableEntry> opcodeResult = decodeOpcode(currentByte);
		if (!opcodeResult)
		{
			std::cerr << std::format("The byte did not map to a valid 8086 instruction::{}\n", std::bitset<8>(currentByte).to_string());
			exit(1);
		}
		InstructionTableEntry entry = *opcodeResult;

		// Begin filling in instruction details
		Instruction instruction = { 0 };
		Decode(instruction, entry, cpu);

		// Print instruction
		decodedInstructions.push_back(instruction);

		// PC currently points to the last byte that was decoded, increment to point to next instruction
		cpu.PC++;

	}

	return decodedInstructions;
}


int main(int argc, char* argv[])
{
	// No filepath is present
	if (argc < 2)
	{
		std::cout << "No input file[s] found." << std::endl;
		return 1;
	}

	// Read binary file into a vector if bytes
	readBinaryFile(argv[1]);

	// Begin decoding bytes one at a time
	std::vector<Instruction> instructions;
	std::vector<Instruction> inst = beginDecode();
	instructions.insert(instructions.end(), inst.begin(), inst.end());

	std::vector<std::string> formattedInstructions;

	for (auto inst : instructions)
	{
		formattedInstructions.push_back(formatInstruction(inst));
	}

	writeToFile(formattedInstructions);

	return 0;
}
