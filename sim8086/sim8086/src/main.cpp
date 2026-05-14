// sim8086.cpp : Defines the entry point for the application.

#include "sim8086.h"

#include <fstream>
#include <bitset>
#include <format>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>


/**
 * @brief Memory for the Intel 8086 simulator. Intel 8086 computers typically had 1 MB of addressable memory. Therefore this simulator is initialized with 1 MB of memory space. 
 */
uint8_t Memory[1024 * 1024];

/**
 * @brief Struct that represents the state of the CPU, including memory and registers
 * @note IP is not in the register table because it is accessed often. This is more for programmer UX to prevent typing array element access syntax constantly
 */
struct CPU {
	uint16_t IP;
	uint16_t registers[8];
};

/**
 * @brief Retrieve a byte from the specified memory address.
 */
uint8_t GetByteFromMemory(uint16_t &addr) {
	uint8_t byte = Memory[addr];
	return byte;
}

/**
 * @brief Retrieve the byte from program memory at the instruction pointer and increment the instruction pointer. 
 * @param ip Instruction pointer 
 * @return Byte from program memory
 */
uint8_t GetNextInstByte(uint16_t &ip)
{
	uint8_t byte = Memory[ip];
	ip++;
	return byte;
}

struct Program {
	uint32_t size;
	uint32_t startAddr;
	uint32_t endAddr;
};

enum OperandMode {
	REGISTER,
	DIRECT_ADDRESS,
	IMMEDIATE,
	EFFECTIVE_ADDRESS_CALC
};


enum Mnemonic {
	MOV, 
	ADD,
	ADC,
	SUB
};

std::string MnemonicToString(uint8_t mnemonicCode)
{
	switch (mnemonicCode)
	{
		case MOV: return "MOV";
		case ADD: return "ADD";
		case ADC: return "ADC";
		case SUB: return "SUB";
	}
}

/**
 * Sentinel value for not in use
 */
#define NA 0xFF

/**
 * Possible width field values
 */
#define WIDE 1
#define BYTE 0

/**
 * Possible operand encoding types 
 */
#define REGISTER 0 // Operand is a register 
#define IMMEDIATE_OR_ADDR 1 // Operand is an immediate value or an address
#define EFFECTIVE_ADDR 2 // Operand is an effective address calculation without displacement
#define EFFECTIVE_ADDR_DIS 3 // Operand is an effective address calculation with displacement


/**
 * @brief Represents a decoded instructions
 * @note This struct totals 15 bytes in memory, most likely will get padded to 16 bytes. 
 */
struct DecodedInstruction
{
	uint8_t opcode;			// Intel 8086 opcode representing a unique instruction and mnemonic 
	uint8_t srcType; 		// Identifies the type of operand SRC is (register, immediate, effective address calculation)
	uint8_t srcBaseReg;		// SRC operand base register 
	uint8_t srcIndexReg;	// SRC operand index register (if present, not all instructions have an index register value) 
	uint8_t destType; 		// Identifies the type of operand DEST is (register, immediate, effective address calculation)
	uint8_t destBaseReg;	// DEST operand base register
	uint8_t destIndexReg;	// DEST operand index register (if present)
	uint16_t address;		// 2-byte value representing a memory address
	uint16_t displacement;	// 2-byte displacement value for effective address calculations
	uint16_t immediate;		// 2-byte immediate value for operations on immediate values
};

/**
 * @brief Identifies what type of encoding an Intel 8086 instruction uses. 
 */
enum EncodingCategory {
	ENCODING_TWO_BYTE_LOGIC,
	ENCODING_ONE_BYTE_ACCUMULATOR,
	ENCODING_TWO_BYTE_LOGIC_IMMEDIATE,
	ENCODING_ONE_BYTE_LOGIC_IMMEDIATE,
	ENCODING_TWO_BYTE_IMMEDIATE_SIGNED
};

struct InstructionTableEntry
{
	uint8_t opcode;
	uint8_t opcodeMask;
	uint8_t mnemonic;
	uint8_t sMask;
	uint8_t wShift; // Needed as w can be in multiple positions in byte 1.
	uint8_t wMask;
	uint8_t dMask;
	uint8_t modMask;
	uint8_t rmMask;
	uint8_t regShift; // this is needed as reg can exist in different spots in an instruction.
	uint8_t regByte; // Indicates if reg is in byte 1 or 2 of instruction. 
	uint8_t regMask;
	uint8_t constMask;
};

InstructionTableEntry InstructionTable[] = {
	{ .opcode=0x88, .opcodeMask=0xFC, .mnemonic=MOV, .sMask=NA, .wShift=0, .wMask=0x01, .dMask=0x02, .modMask=0xC0, .rmMask=0x07, .regByte=2, .regShift=0x3, .regMask=0x38, .constMask=NA }
};

#define INST_TABLE_SIZE sizeof(InstructionTable) / sizeof(InstructionTableEntry)

/**
 * Table holding human readible names of registers
 * 
 * Accessing register names happens using register index + width.
 * Example: 
 * 	AL is register index 0 and width is 0: registerNames[0 + 0] = AL
 *	AX is register index 0 as well but width of 1: registerNames[0 + 1] = registerNames[1] = AX     	   
 */
const char* RegisterNames[8][2] = {
	{ "AL", "AX" },
	{ "CL", "CX" },
	{ "DL", "DX" },
	{ "BL", "BX" },
	{ "AH", "SP" },
	{ "CH", "BP" },
	{ "DH", "SI" },
	{ "BH", "DI" },
};

/**
 * @brief Search instruction table for instruction that matches the opcode in the byte. 
 * @param byte The byte containing the opcode to search for in the instruction table.
 * @return Returns index of instruction in instruction table if found, otherwise returns -1.
 */
/* CODE REVIEW: This is a linear search. Could be better, we will optimize later. */
std::optional<InstructionTableEntry> FindInstruction(uint8_t byte)
{
	for (int i = 0; i < INST_TABLE_SIZE; i++)
	{
		// Get opcode from byte
		InstructionTableEntry inst = InstructionTable[i];
		uint8_t b = byte & inst.opcodeMask;

		// If byte matches opcode, break out and return index of instruction
		if (b == inst.opcode)
		{
			return inst;
		}
	}

	return std::nullopt;
}

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

std::string FormatInstruction(OperandMode mode, uint8_t width)
{
	switch(mode)
	{
		case REGISTER:
		{
		} break;
		case IMMEDIATE:
		{
		} break;
		case EFFECTIVE_ADDRESS_CALC:
		{

		} break;
		case DIRECT_ADDRESS:
		{
			
		} break;
	}
}

/** 
TODO: Needs to be updated to match new instruction format
*/
std::string FormatSize()
{
	
}


/* CODE REVIEW: We should be batching these formated instructions and flushing out the buffer once it gets full to prevent constant OS calls and blocking threads */
void WriteToFile(DecodedInstruction instruction, std::ofstream &file, bool flush = false)
{
	std::string src; /** TODO: */
	std::string dest; /** TODO: */

	std::string size; /** TODO: */

	/** TODO: Write to file */
}


/**
 * @brief Read a 16-bit little-endian word from `memory` starting at address `PC`.
 *
 * The byte at `memory[PC]` is treated as the low byte and the byte at
 * `memory[PC + 1]` is treated as the high byte. These are combined into a
 * 16-bit value `(high_byte << 8) | low_byte` which is returned.
 *
 * Note: `PC` is passed by reference and is modified by this function.
 *
 * @param memory Array representing emulator memory.
 * @param PC Address of the low byte to read.
 * @return 16-bit unsigned integer composed as `(high_byte << 8) | low_byte`.
 * @note Accessing `memory[PC]` or `memory[PC + 1]` out of bounds is undefined
 *       behavior; callers must ensure `PC` and `PC + 1` are valid addresses.
 */
int16_t LoadWordData(struct CPU &cpu)
{
	uint8_t lowByte = GetNextInstByte(cpu.IP);
	uint8_t highByte = GetNextInstByte(cpu.IP);

	int16_t data = static_cast<int16_t>(highByte);
	data = (data << 8) | lowByte;
	return data;
}

/**
 * @brief Read an 8-bit byte from `memory` at address `PC` and return it as a 16-bit unsigned integer.
 * @param cpu The CPU structure containing memory and PC.
 * @return The data loaded from memory as an unsigned 16-bit integer (upper bits zeroed).
 * @note This function relates to 8086 immediate byte operands, where 8-bit values are loaded from the instruction stream.
 */
int16_t LoadByteData(struct CPU &cpu)
{
	uint8_t byte = GetNextInstByte(cpu.IP);
	int8_t byteSigned = static_cast<int8_t>(byte);
	return static_cast<int16_t>(byteSigned);
}

/**
 * @brief Extracts the MOD field from the current opcode byte.
 * @param modMask Bitmask to isolate the MOD bits (typically 0xC0).
 * @param cpu The CPU structure.
 * @return The 2-bit MOD value (0-3), indicating addressing mode.
 * @note In 8086, MOD (bits 7-6 of MOD-RM byte) specifies memory/register mode: 00=mem no disp, 01=mem 8-bit disp, 10=mem 16-bit disp, 11=register.
 */
uint8_t DecodeMod(uint8_t modMask, uint8_t byte)
{
	return (byte & modMask) >> 6;
}

/**
 * @brief Extracts the RM field from the current opcode byte.
 * @param rmMask Bitmask to isolate the RM bits (typically 0x07).
 * @param cpu The CPU structure.
 * @return The 3-bit RM value (0-7), indicating register or effective address base.
 * @note In 8086, RM (bits 2-0 of MOD-RM byte) selects the register or base for addressing (e.g., BX+SI).
 */
uint8_t DecodeRm(uint8_t rmMask, uint8_t byte)
{
	return (byte & rmMask);
}

/**
 * @brief Extracts the REG field from the current opcode byte and formats the register mnemonic.
 * @param regMask Bitmask to isolate the REG bits.
 * @param regShift Number of bits to shift right to get the REG value.
 * @param instruction The instruction structure to store the mnemonic.
 * @param cpu The CPU structure.
 * @return The 3-bit REG value (0-7).
 * @note In 8086, REG (bits 5-3 of MOD-RM byte) specifies the source/destination register in reg-reg/mem operations.
 */
uint8_t DecodeReg(uint8_t regMask, uint8_t regShift, uint8_t byte)
{
	return (byte & regMask) >> regShift;
}

/**
 * @brief Extracts a constant bit field from the current opcode byte.
 * @param constMask Bitmask to isolate the constant bits.
 * @param cpu The CPU structure.
 * @return The extracted bit value, shifted to the least significant position.
 * @note In 8086 arithmetic instructions, this extracts specific control bits from the opcode.
 */
uint8_t DecodeBitConst(uint8_t constMask, uint8_t byte)
{
	return (byte & constMask) >> 3;
}


/**
 * When direction == 0, operandA goes into src 
 * When direction == 1, operandB goes into src
 */
void InterpretOperandDirection()
{
	// TODO? 
}



/**
 * @brief Reads binary file into memory
 * @param filePath
 * @param buffer
 * @return An instance of Program that contains metadata about the program loaded into memory
 */
Program LoadProgramIntoMemory(std::string filePath)
{
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);

	if (!file.is_open() || errno == ENOENT)
	{
		std::cerr << "ERROR: Could not open file. File does not exist.\n";
		return { 0 };
	}

	// Determine file size
	file.seekg(0, file.end);
	uint32_t length = static_cast<uint32_t>(file.tellg());
	file.seekg(0, file.beg);

	/**
		TODO: This is not in its final form. Ultimately, we would need some function that finds 
			a block of open memory large enough, gets the starting address & ending address, etc. 
			Essentially, we need proper memory management here. I am just doing this for short term
			to test out this strategy. 
	*/
	Program program = { 0 };
	file.read(reinterpret_cast<char*>(Memory), length);

	program.size = length;
	program.startAddr = 0;
	program.endAddr = length;
	return program; 
}

/**
 * When we execute a program, we should expect output, i.e. specific CPU state. This should be much more testable now
*/
void Execute(struct CPU &cpu)
{
	// TODO....
}

/**
 * @brief Main decoding function that dispatches to specific decoders based on instruction category.
 * @param instruction The instruction structure to populate with decoded information.
 * @param entry The instruction table entry containing mnemonic and encoding details.
 * @param cpu The CPU structure; PC is advanced during decoding.
 * @note This function emulates the 8086's instruction decoding pipeline, routing opcodes to appropriate microcode-like handlers.
 */
void Decode(InstructionTableEntry& entry, DecodedInstruction &decodedInst, struct CPU& cpu)
{
	decodedInst.opcode = entry.opcode;
	uint8_t currentByte = GetByteFromMemory(cpu.IP);

	uint8_t direction;
	if (entry.dMask != NA) direction = (entry.dMask & currentByte) >> 1;

	uint8_t width;
	if (entry.wMask != NA) width = (entry.wMask & currentByte);

	uint8_t sign;
	if (entry.sMask != NA) sign = (entry.sMask & currentByte) >> 1;

	if (entry.regByte == 2) currentByte = 

	uint8_t reg;
	if (entry.regMask != NA) reg = (entry.regMask & currentByte) >> entry.regShift;

}

/**
 * @brief Disassemble an Intel 8086 binary into Intel 8086 assembly code
 */
/* CODE REVIEW: This should probably return something that can indicate a success or failure */
void Disassemble(Program &program)
{
	struct CPU cpu = { 0 };
	/* CODE REVIEW: We shouldn't hard code the file name, this should be passed in by the user or default to the name of the file being disassembled */
	std::ofstream file = OpenAsmFile("/Users/joey/Projects/sim-8086/sim8086/sim8086/tests/result.asm");

	while (cpu.IP < program.size)
	{
		uint8_t currentByte = GetByteFromMemory(cpu.IP);

		// Get opcode from byte
		/* CODE REVIEW: I need to dig deeper into how std::optional works, I feel like this is overkill and probably can be accomplished with simple pointers. This would avoid the dereferencing below. */
		std::optional<InstructionTableEntry> opcodeResult = FindInstruction(currentByte);
		if (!opcodeResult)
		{
			std::cerr << std::format("The byte did not map to a valid 8086 instruction::{}\n", std::bitset<8>(currentByte).to_string());
			return;
		}

		InstructionTableEntry entry = *opcodeResult;
		DecodedInstruction decodedInst = { 0 };
		Decode(entry, decodedInst, cpu);
 
		WriteToFile(decodedInst, file);
	}

	CloseAsmFile(file);
}


/**
 * TODO: This could be enhanced with a feature to specify what action a user is trying to accomplish. 
 * For example:
 * 	-d: decode
 * 	-s: simulate
 * 	-o: output file name 
 * 	etc. 
 * 
 * TODO: Each run of the simulator will have one responsibility for now
 * 	- execute
 *  - disassemble 
 * We are just focusing on disassembling for now 
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
	std::string asmFile = argv[1];
	struct Program program = LoadProgramIntoMemory(asmFile);

	// For now we are just disassembling.
	Disassemble(program);

	return 0;
}
