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
	Reg_bit,
	Rm_bit,
	Mod_bit,
	Data_bit,
	Addr_bit,

	Field_count
};

enum ModCategory: uint8_t 
{
	Memory_mode_no_disp,
	Memory_mode_8_bit_disp,
	Memory_mode_16_bit_disp,
	Register_mode,

	Mod_category_count
};

enum EffectiveAddressBase: uint8_t 
{
	Effective_addr_direct_address,

	Effective_addr_bx_si,
	Effective_addr_bx_di,
	Effective_addr_bp_si,
	Effetive_addr_bp_di,
	Effective_addr_si,
	Effective_addr_di,
	Effective_addr_bx,

	Effective_addr_count
};

struct EffectiveAddrExpression
{
	EffectiveAddressBase base;
	int16_t displacement;
};

/**
 * Represents the bit patterns/fields in an Intel 8086 instruction
 */
struct Bits 
{
	Field field;
	uint8_t value;
	uint8_t shift;
	uint8_t count;
};

struct InstEntry {
	const char* mnemonic;
	Bits bits[16];	// bits[0] = Opcode bits
};

InstEntry InstructionTable[] = {
	#include "InstructionTable.inl"
};

/* Operation Definitions */

enum Operation: uint8_t {

    #define INST(mnemonic, ...) Op_##mnemonic
    #define INST_ALT(...)
	#include "InstructionTable.inl"

	Op_count
};

struct DecodedInst {
	Operation op;
	Bits bits[16];
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

void Execute(struct CPU &cpu)
{
	// TODO....
}

DecodedInst Decode(CPU &cpu, InstEntry entry) 
{
	DecodedInst inst = {};
	uint8_t byte = GetCurrentByte(cpu.IP);

	// Get Opcode field 
	printf("Op: %x", entry.bits[0].value);
	printf("\n");

	// Instruction opcode matched, begin decode
	uint8_t bitsIndex = 1;
	uint8_t usedBits = entry.bits[0].count;;

	while (!IsBitsDefined(entry.bits[bitsIndex]))
	{

		if (usedBits >= 8) byte = GetNextByte(cpu.IP);

		Bits bit = entry.bits[bitsIndex];
	
	}

	return inst;
}


/**
 * Instruction Table should drive decode, not vice versa
 * An entry tells you how many bits/bytes to pull from the stream
 */
void Disassemble(Program &program)
{	
	CPU cpu = { 0 };

	while (cpu.IP <= program.endAddr)
    {
        uint8_t currentByte = GetNextByte(cpu.IP);

		InstEntry entry = {};
        // Search Instruction table for matching instruction 
        for (int i = 0; i < ArrayCount(InstructionTable); i++)
        {
            InstEntry entry = InstructionTable[i];

			// Break out of loop if matching opcode is found
            if (entry.bits[0].value == (currentByte >> entry.bits[0].shift))
			{
				break;
			}

        }

		DecodedInst inst = Decode(cpu, entry);
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
