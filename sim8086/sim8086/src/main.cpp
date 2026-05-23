// sim8086.cpp : Defines the entry point for the application.

#include <stdint.h>
#include <fstream>
#include <iostream>

/**
 * Bit Stream Approach
 * -------------------
 * 
 */

#define ArrayCount(array) sizeof(array)/sizeof(array[0])


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

enum Field : uint8_t
{
	Literal,
	W_bit,
	D_bit,

	Field_count
};

/**
 * Represents the bit patterns/fields in an Intel 8086 instruction
 */
struct Bits 
{
	// Name of the field being encoded (W, D, literal, etc.)
	Field field;
	// Bit literal if exists (i.e. Opcode bit pattern, const bit pattern, etc. )
	uint8_t value;
	// Amount to shift if required 
	uint8_t shift;
	// The number of bits in the entry (i.e. the `w` field has count of 1)
	uint8_t count;
};

struct InstEntry {
	const char* mnemonic;
	// Bit fields contain in the instruction. Entry 0 is always opcode 
	Bits bits[16];
};

InstEntry InstructionTable[] = {
	#include "InstructionTable.inl"
};

/* Operation Definitions */
#define INST (mnemonic, ...) mnemonic
enum Operation {

	#include "InstructionTable.inl"

	Op_count
};
#undef INST

struct ParsedInst {
	Operation op;
	Bits bit[16];
};

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
 * Used to determine if all bits in an instruction have been decoded. 
 * If all fields are 0, then its an uninitialized struct and is the end
 * of the bits array. 
 */
bool IsBitsDefined(Bits bits)
{
	return bits.field == Literal
		&& bits.count == 0
		&& bits.shift == 0
		&& bits.value == 0;
}

/**
 * Instruction Table should drive decode, not vice versa
 * An entry tells you how many bits/bytes to pull from the stream
 */
void Disassemble(Program &program)
{	
	CPU cpu = { 0 };
	uint8_t currentByte = GetNextByte(cpu.IP);

	while (currentByte != program.endAddr)
    {
        // Search Instruction table for matching instruction 
        for (int i = 0; i < ArrayCount(InstructionTable); i++)
        {
            InstEntry entry = InstructionTable[i];
			// Check if opcode of instruction matches byte
            if (entry.bits[0].field == (currentByte >> entry.bits[0].shift))
			{
				// Instruction opcode matched, begin decode

				uint8_t bitsIndex = 0;
				uint8_t usedBits = 0;
				while (!IsBitsDefined(entry.bits[bitsIndex]))
				{
					Bits bit = entry.bits[bitsIndex];
					switch(bit.field)
					{
						case Literal:
						{
							printf("Literal: %x", bit.value);
							printf("\n");
						}break;
						case W_bit:
						{
							printf("W: %d", bit.)
						}break;
						case D_bit:
						{

						} break;
						default:
						{

						}
					}
				}
			}

        }
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
