// sim8086.cpp : Defines the entry point for the application.

#include "sim8086.h"

#include <stdint.h>
#include <fstream>
#include <iostream>

/**
 * Bit Stream Approach
 * -------------------
 * 
 */

// End of Program 
#define EOP -1

/** Instruction Encoding Definitions */
#define INST(opcode, mnemonic, ...) { opcode, #mnemonic, __VA_ARGS__}



uint8_t Memory[1024 * 1024];

struct CPU {
	uint16_t IP;
	uint16_t registers[8];
};

struct Program {
	uint32_t size;
	uint32_t startAddr;
	uint32_t endAddr;
};

/**
 * Get the next byte from the code segment in memory and increment 
 * the instruction pointer (IP).
 */
uint8_t GetNextByte(uint16_t &ip)
{
	return Memory[ip++];
}

/**
 * Get the current byte in the code segment of memory that IP points too. 
 * Note: Does not increment IP. 
 */
uint8_t GetCurrentByte(uint16_t &ip)
{
	return Memory[ip];
}

struct InstEntry {
	uint8_t opcode;
	const char* mnemonic;
};

/* Instruction Table */
#define Instructions \
	INST(0b100010, MOV)

InstEntry InstructionTable[] = {
	Instructions
};

#undef Instructions

std::ofstream OpenAsmFile(std::string name)
{
	std::ofstream asmFile;
    asmFile.open(name);

	// TODO: Check if file failed to open.
	if (!asmFile.is_open())
	{
		std::cerr << "Could not open file" << std::endl;
		return asmFile;
	}

	// Write header of asm file 
	asmFile << "bits 16\n\n";
	return asmFile;
}

void CloseAsmFile(std::ofstream &file)
{
	file.close();
}

void WriteToFile()
{
	std::string src; /** TODO: */
	std::string dest; /** TODO: */

	std::string size; /** TODO: */

	/** TODO: Write to file */
}

Program LoadProgramIntoMemory(std::string filePath)
{
	// Open binary file, at the end (ate)
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);

	if (!file.is_open() || errno == ENOENT)
	{
		std::cerr << "ERROR: Could not open file. File does not exist.\n";
		return { 0 };
	}

	// Get file size
	uint32_t length = static_cast<uint32_t>(file.tellg());
	file.seekg(0, file.beg);
	file.read(reinterpret_cast<char*>(Memory), length);

	Program program = {
		.size=length,
		.startAddr=0,
		.endAddr=length - 1
	};
	return program; 
}

void Execute(struct CPU &cpu)
{
	// TODO....
}

void Decode()
{
	// TODO....
}

/**
 * Instruction Table should drive decode, not vice versa
 * An entry tells you how many bits/bytes to pull from the stream
 */
void Disassemble(Program &program)
{	
	CPU cpu = { 0 };
	uint8_t currentByte = GetNextByte(cpu.IP);

	/**
	 * TODO: Not a great way to determine program end. We should be looking at the 
	 * end address or something else. This is just a stop gap to get code flowing. 
	 */
	while(cpu.IP < program.size)
	{
		// Iterate instruction table looking for the correct instruction. 
	}
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "No input file[s] found." << std::endl;
		return 1;
	}

	std::string asmFile = argv[1];
	struct Program program = LoadProgramIntoMemory(asmFile);

	Disassemble(program);

	return 0;
}
