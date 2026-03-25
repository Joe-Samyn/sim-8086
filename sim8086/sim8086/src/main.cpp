// sim8086.cpp : Defines the entry point for the application.

#include "sim8086.h"

#include <fstream>
#include <bitset>
#include <format>
#include <vector>
#include <unordered_map>
#include <string>

/*
TODO: 
- Remove a global CPU variable, make it run specific. This will cause issues when unit
	testing is added or any kind of asynchronous systems. It will require mutex locks, 
	or complex logic to manage
- The instruction formating for printing to file seems bad. We should revisit this and
	see if there is a cleaner way to do this. 
- Stop incrementing PC everywhere, have a single function that increments the PC or gets
	the next byte. This should probably live in the Hardware.h file since its directly 
	affecting a hardware item 
- This simulator is built with a mix of decoding and simulating structure. The parts that
	really only support decoding are going to have to be redone/rethought. Example would be
	the effective address calculations and registers. These mainly just convert to strings. 
	When we get to simulating, we will need to perform actual operations with these. 
- All code primarily resides in header files. This should be refactored so that implementations
	are in cpp files. It will help with debugging and organization. Also more standard
- 
*/

struct CPU cpu = { 0 };

/*
 TODO: This should move into Hardware.h
*/
void displayCpuState()
{
    std::cout << std::format("PC: {}\nCurrent Byte: {}\n", cpu.PC, std::bitset<8>(cpu.memory[cpu.PC]).to_string());
}

/*
	TODO: We can get more granular on which instructions need to have size defined (word, byte) instead of doing all of them
*/
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
 * TODO: This should move to somewhere that deals with loading a program like FileIO or something
 * TODO: We need to keep track of file size. The HLT instruction being used as a terminating instruction is
 * not going to work long term. 
 */
void readBinaryFile(char* filePath)
{
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);

	/**
	 * TODO: Not proper file error handling. Should be || and handle all failures to open
	 */
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

/**
 * TODO: This may not be realistic. Chances are, simulating is not going to require entire programs
 * to be decoded. The instructions will be decoded one at a time and the instruction will be executed. 
 * This should be adjusted to just be a specific feature of the sim8086 application. 
 * 
 * TODO: This function is not the most testable. Its essentially a full decode integration test. 
 * We should think about ways to make this more deterministic and testable. 
 */
std::vector<Instruction> beginDecode()
{
	std::vector<Instruction> decodedInstructions = {};
	/**
	 * TODO: Not the biggest fan of this, could be a lot more readable
	 */
	while (cpu.memory[cpu.PC] != HALT)
	{
		/**
		 * TODO: We are already doing an array access above to get the current byte, seems redundant to do it again
		 */
		uint8_t currentByte = cpu.memory[cpu.PC];

		// Get opcode from byte
		/**
		 *  TODO: This could be cleaned up by having a global error function
		 * 	TODO: Probably should be specific decodeOpcodeByte rather than just decodeOpcode. I bet we could simplify a lot of
		 * 	the logic in other microcode functions. There should then be a specific action to findInstruction or something. 
		 * 	Two very testable, single responsibility functions. 
		 *  TODO: Need to remove exit(1). This will not work in unit testing scenarios. It breaks out of the program. 
		 */
		std::optional<InstructionTableEntry> opcodeResult = decodeOpcode(currentByte);
		if (!opcodeResult)
		{
			std::cerr << std::format("The byte did not map to a valid 8086 instruction::{}\n", std::bitset<8>(currentByte).to_string());
			exit(1);
		}
		/**
		 * TODO: Instruction table is globally accessible and doesn't change, we probably don't need to pass around this entry 
		 * as a parameter everywhere. 
		 */

		InstructionTableEntry entry = *opcodeResult;

		/**
		 * TODO: Again, we could probably do this above and get multiple things out of the decodeOpcodeByte function
		 */
		Instruction instruction = { 0 };
		Decode(instruction, entry, cpu);

		/**
		 * TODO: We have an entire data structure to keep track of all the decoded instructions, this is probably unnecessary. 
		 * We should be able to just output an instruction to a file or buffer once we have it. This might even be better off
		 * as a string[] in the FileIO section of the code base. That is probably a lot simpler and more performant than a vector. 
		 */
		decodedInstructions.push_back(instruction);

		// PC currently points to the last byte that was decoded, increment to point to next instruction
		/**
		 * TODO: We should have a single function in Hardware.h that handles incrementing the PC or getting the next byte. 
		 * Having this operationg everywhere is asking for bugs. 
		 */
		cpu.PC++;

	}

	/**
	 * TODO: May not need this if we are going to change how we output instructions
	 */
	return decodedInstructions;
}

/**
 * TODO: This could be enhanced with a feature to specify what action a user is trying to accomplish. 
 * For example:
 * 	-d: decode
 * 	-s: simulate
 * 	-o: output file name 
 * 	etc. 
 */
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

	/**
	 * TODO: May be unnecessary. Just seems like a lot of overhead
	 */
	std::vector<Instruction> instructions;

	/**
	 * TODO: Since decoding will be a feature of the overall simulator, rename beginDecode to just decode() and run decode loop. 
	 */
	std::vector<Instruction> inst = beginDecode();
	instructions.insert(instructions.end(), inst.begin(), inst.end());

	/**
	 * TODO: Very unnecessary and exessive. Should be accomplished much simpler 
	 */
	std::vector<std::string> formattedInstructions;

	/**
	 * TODO: Seems like an unnecessary loop. Could be done as part of the normal decode loop rather than in main. 
	 */
	for (auto inst : instructions)
	{
		formattedInstructions.push_back(formatInstruction(inst));
	}

	writeToFile(formattedInstructions);

	return 0;
}
