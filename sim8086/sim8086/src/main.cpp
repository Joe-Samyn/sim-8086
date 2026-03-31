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
 * TODO: We need to define a register table to look up the human friendly name for these registers. 
 */


/**
 * @brief Retrieve a byte from a prgram in memory at the specified Instruction Pointer. 
 * The instruction pointer is incremented by 1 after the byte is retrieved from memory.
 */
uint8_t GetInstructionByte(uint16_t &ip, bool incrementIp = false) {
	uint8_t byte = Memory[ip];
	if (incrementIp) ip++;
	return byte;
}

struct Program {
	uint32_t size;
	uint32_t startAddr;
	uint32_t endAddr;
};

enum OperandType {
	REGISTER,
	DIRECT_ADDRESS,
	IMMEDIATE,
	EFFECTIVE_ADDRESS_CALC,
	EFFECTIVE_ADDRESS_CALC_W_DISPLACEMENT
};

enum Mnemonic {
	MOV, 
	ADD,
	ADC,
	SUB
};

std::string MnemonicToString(Mnemonic m)
{
	switch (m)
	{
		case MOV: return "MOV";
		case ADD: return "ADD";
		case ADC: return "ADC";
		case SUB: return "SUB";
	}
}

/**
 * @note The registers are uint8_t because they represent the index of the register in the CPUs register table (array)
 */
struct Operand {
	uint8_t baseRegister;
	uint8_t indexRegister;
	uint16_t displacement;
	uint16_t directAddress;
	int16_t immediate;
};

/**
 * @brief Represents a decoded instructions
 */
struct Instruction
{
	Mnemonic mnemonic;

	uint8_t direction;
	uint8_t width;

	OperandType operandAType;
	Operand operandA;

	OperandType operandBType;
	Operand operandB;
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


struct TwoByteLogicEntry
{
	uint8_t dMask;
	uint8_t wMask;
	uint8_t modMask;
	uint8_t regMask;
	uint8_t regShift;
	uint8_t rmMask;
};

struct TwoByteLogicImmediateEntry
{
	uint8_t wMask;
	uint8_t modMask;
	uint8_t rmMask;
};

struct OneByteAccumulatorEntry
{
	uint8_t direction;
	uint8_t wMask;
	bool hasAddress;
};

struct OneByteLogicImmediateEntry
{
	uint8_t wMask;
	uint8_t wShift;
	uint8_t regMask;
};

struct TwoByteImmedSignedEntry 
{
	uint8_t sMask;
	uint8_t wMask;
	uint8_t modMask;
	uint8_t rmMask;
	uint8_t constMask;
	uint8_t direction;
};

struct InstructionTableEntry
{
	uint8_t opcode;
	uint8_t opcodeMask;
	Mnemonic mnemonic;
	EncodingCategory category;
	union
	{
		struct TwoByteLogicEntry twoByteLogicEntry;
		struct TwoByteLogicImmediateEntry twoByteLogicImmediateEntry;
		struct OneByteAccumulatorEntry threeByteAccumulatorEncoding;
		struct OneByteLogicImmediateEntry oneByteLogicImmediateEncoding;
		struct TwoByteImmedSignedEntry arithmeticTwoByteImmedSignedEntry;

	} encoding;
};

// TODO (joe): Maybe remove strings from this table and create a mapping table of opcode -> mnemonic
#define InstructionTableEntries \
	X(0x88, 0xFC, MOV, ENCODING_TWO_BYTE_LOGIC, 0x02,  0x01, 0xC0, 0x38, 0x3, 0x07), \
	X(0xC6, 0xFE, MOV, ENCODING_TWO_BYTE_LOGIC_IMMEDIATE, 0x01, 0xC0, 0x07), \
	X(0xB0, 0xF0, MOV, ENCODING_ONE_BYTE_LOGIC_IMMEDIATE, 0x08, 0x03, 0x07), \
	X(0xA0, 0xFE, MOV, ENCODING_ONE_BYTE_ACCUMULATOR, 0x01, 0x01, true), \
	X(0xA2, 0xFE, MOV, ENCODING_ONE_BYTE_ACCUMULATOR, 0x00, 0x01, true), \
	X(0x00, 0xFC, ADD, ENCODING_TWO_BYTE_LOGIC, 0x02,  0x01, 0xC0, 0x38, 0x3, 0x07), \
	X(0x80, 0xFC, ADD, ENCODING_TWO_BYTE_IMMEDIATE_SIGNED,  0x02, 0x01, 0xC0, 0x07, 0x38, 0x01), \
    X(0x04, 0xFE, ADD, ENCODING_ONE_BYTE_ACCUMULATOR, 0x01, 0x01, false), \
	X(0x14, 0xFE, ADC, ENCODING_ONE_BYTE_ACCUMULATOR, 0x01, 0x01, false), \
	X(0x10, 0xFC, ADC, ENCODING_TWO_BYTE_LOGIC, 0x02,  0x01, 0xC0, 0x38, 0x3, 0x07), \
	X(0x28, 0xFC, SUB, ENCODING_TWO_BYTE_LOGIC, 0x02,  0x01, 0xC0, 0x38, 0x3, 0x07), \
	X(0x2C, 0xFE, SUB, ENCODING_ONE_BYTE_ACCUMULATOR, 0x01, 0x01, false) \

#define X(opcode, opcodeMask, mnemonic, category, ...) { opcode, opcodeMask, mnemonic, category, { __VA_ARGS__ } }
std::vector<InstructionTableEntry> instructionTable = {
	InstructionTableEntries
};
#undef X

/**
 * @brief Search instruction table for instruction that matches the opcode in the byte. 
 * @param byte The byte containing the opcode to search for in the instruction table.
 * @return Returns index of instruction in instruction table if found, otherwise returns -1.
 */
std::optional<InstructionTableEntry> FindInstruction(uint8_t byte)
{
	for (int i = 0; i < instructionTable.size(); i++)
	{
		// Get opcode from byte
		InstructionTableEntry inst = instructionTable.at(i);
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

	// Write header of asm file 
	asmFile << "bits 16\n\n";
}

void CloseAsmFile(std::ofstream file)
{
	file.close();
}

void WriteToFile(Instruction instruction, std::ofstream file)
{
	switch (instruction.mnemonic)
	{
		case MOV:
		{
			if (instruction.operandAType == REGISTER && instruction.operandBType == REGISTER)
			{
				file << std::format("{} {}, {}", MnemonicToString(instruction.mnemonic), )
			}
		} break;
	}
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
uint16_t LoadWordData(struct CPU &cpu)
{
	uint8_t lowByte = GetInstructionByte(cpu.IP, true);
	uint16_t highByte = static_cast<uint16_t>(GetInstructionByte(cpu.IP));

	uint16_t wideValue = (highByte << 8) | lowByte;
	return wideValue;
}

/**
 * @brief Read an 8-bit byte from `memory` at address `PC` and return it as a 16-bit unsigned integer.
 * @param cpu The CPU structure containing memory and PC.
 * @return The data loaded from memory as an unsigned 16-bit integer (upper bits zeroed).
 * @note This function relates to 8086 immediate byte operands, where 8-bit values are loaded from the instruction stream.
 */
int16_t LoadByteData(struct CPU &cpu)
{
	int8_t lowByte = static_cast<int8_t>(Memory[cpu.IP]);
    int16_t data = static_cast<int16_t>(lowByte);
	return data;
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
 * @brief Decodes the MOD field and performs microcode operations for the specific mod encodings as specified in the 8086 manual.
 * @param mod The MOD value (0-3).
 * @param rm The RM value (0-7).
 * @param instruction The instruction structure to update with the mnemonic.
 * @param cpu The CPU structure; PC is advanced as needed.
 * @note This function emulates 8086's microcode for parsing effective addresses, handling displacements and register modes.
 */
void InterpretModField(uint8_t mod, uint8_t rm, Instruction& instruction, struct CPU &cpu)
{
	switch (mod)
	{
	case 0x00: // Memory Mode, No Displacement
	{
		// Special case of MOD that indicates direct memory access with no displacement. 
		if (rm == 0x6)
		{
			instruction.operandBType = DIRECT_ADDRESS;
			instruction.operandB.directAddress = LoadWordData(cpu);
		}	
		else
		{
			instruction.operandBType = REGISTER;
			instruction.operandB.baseRegister = rm;
		}	
	} break;
	case 0x01: // Memory Mode, 8-bit displacement
	{
		
	} break;
	case 0x02: // Memory Mode, 16-bit displacement
	{
		
	} break;
	case 0x03: // Register Mode, No Displacement
	{
        instruction.operandBType = REGISTER;
        instruction.operandB.baseRegister = rm;
	} break;
	}
}

/**
 * @brief Decodes two-byte logic instructions (e.g., MOV reg/mem to reg/mem).
 * @param instruction The instruction structure to populate.
 * @param entry The instruction table entry with encoding details.
 * @param cpu The CPU structure; PC is advanced.
 * @note Handles 8086 instructions like MOV, ADD with MOD-RM and REG operands, extracting width, direction, and operands.
 */
void DecodeTwoByteLogic(Instruction& instruction, InstructionTableEntry& entry, struct CPU& cpu)
{
	struct TwoByteLogicEntry logicEntry = entry.encoding.twoByteLogicEntry;
	uint8_t currentByte = GetInstructionByte(cpu.IP, true);

	// Get width if w bit is present 
	if (logicEntry.wMask != 0)  instruction.width = (currentByte & logicEntry.wMask);

	// Get direction if d bit is present, direction is always in bit 1 if it is present so we can just shift right by 1 to get the value
	if (logicEntry.dMask != 0) instruction.direction = (currentByte & logicEntry.dMask) >> 1;

	currentByte = GetInstructionByte(cpu.IP);
	if (logicEntry.regMask != 0) 
	{
		instruction.operandA.baseRegister = DecodeReg(logicEntry.regMask, logicEntry.regShift, currentByte);
		instruction.operandAType = REGISTER;
	}
	
	uint8_t mod = DecodeMod(logicEntry.modMask, currentByte);
	uint8_t rm = DecodeRm(logicEntry.rmMask, currentByte);
	InterpretModField(mod, rm, instruction, cpu);
}

/**
 * @brief Decodes two-byte logic instructions with immediate operands (e.g., MOV immediate to reg/mem).
 * @param instruction The instruction structure to populate.
 * @param entry The instruction table entry with encoding details.
 * @param cpu The CPU structure; PC is advanced.
 * @note Emulates 8086 microcode for instructions like MOV immediate, loading the immediate value after MOD-RM.
 */
void DecodeTwoByteLogicImmediate(Instruction& instruction, InstructionTableEntry& entry, struct CPU& cpu)
{
	struct TwoByteLogicImmediateEntry logicImmediateEntry = entry.encoding.twoByteLogicImmediateEntry;

	uint8_t currentByte = GetInstructionByte(cpu.IP, true);
	// Get width if w bit is present 
	if (logicImmediateEntry.wMask != 0) instruction.width = (Memory[cpu.IP] & logicImmediateEntry.wMask);


	currentByte = GetInstructionByte(cpu.IP, true);
	uint8_t mod = DecodeMod(logicImmediateEntry.modMask, currentByte);
	uint8_t rm = DecodeRm(logicImmediateEntry.rmMask, currentByte);
	InterpretModField(mod, rm, instruction, cpu);
	// instruction.immediate = LoadImmediate(instruction.width, currentByte); TODO: Need to update to new style
}

/**
 * @brief Decodes one-byte logic instructions with immediate operands (e.g., MOV immediate to register).
 * @param instruction The instruction structure to populate.
 * @param entry The instruction table entry with encoding details.
 * @param cpu The CPU structure; PC is advanced.
 * @note Handles 8086 instructions like MOV AL/AX, immediate, where the register is encoded in the opcode byte.
 */
void DecodeOneByteLogicImmediate(Instruction& instruction, InstructionTableEntry& entry, struct CPU& cpu)
{
	struct OneByteLogicImmediateEntry logicImmediateEntry = entry.encoding.oneByteLogicImmediateEncoding;

	// Get width if w bit is present 
	instruction.width = (Memory[cpu.IP] & logicImmediateEntry.wMask) >> logicImmediateEntry.wShift;

	uint8_t reg = (Memory[cpu.IP] & logicImmediateEntry.regMask);
	// TODO: Update to new style
	// instruction.regMnemonic = std::format("{}", GetRegister(reg, instruction.width));

	// instruction.immediate = LoadImmediate(instruction.width, cpu);
}

/**
 * @brief Decodes accumulator instructions (e.g., MOV accumulator, memory/immediate).
 * @param instruction The instruction structure to populate.
 * @param entry The instruction table entry with encoding details.
 * @param cpu The CPU structure; PC is advanced.
 * @note Acts like microcode for 8086 accumulator operations, such as MOV AL/AX, [mem] or MOV AL/AX, immediate, where the accumulator is implied.
 */
void DecodeAccumulator(Instruction& instruction, InstructionTableEntry& entry, struct CPU& cpu)
{
	// struct OneByteAccumulatorEntry accumulatorEntry = entry.encoding.threeByteAccumulatorEncoding;
	// instruction.direction = accumulatorEntry.direction;
	// instruction.width = (Memory[cpu.IP] & accumulatorEntry.wMask);
    // if (accumulatorEntry.hasAddress)
    //     instruction.address = LoadImmediate(1, cpu);
    // else
        
    //     instruction.immediate = LoadImmediate(instruction.width, cpu);
    
	// instruction.regMnemonic = std::format("{}", GetRegister(0x00, instruction.width));
    
}

void DecodeArithmeticTwoByteSigned(Instruction& instruction, InstructionTableEntry& entry, struct CPU& cpu)
{
	// TwoByteImmedSignedEntry signedEntry = entry.encoding.arithmeticTwoByteImmedSignedEntry;

	// uint8_t s = (Memory[cpu.IP] & signedEntry.sMask) >> 1;
	// instruction.width = Memory[cpu.IP] & signedEntry.wMask;
	// instruction.sign = Memory[cpu.IP] & signedEntry.sMask;

	// // Increment because all the following data is in byte 2
	// cpu.IP++;
	// uint8_t mod = GetMod(signedEntry.modMask, cpu);
	// uint8_t rm = GetRm(signedEntry.rmMask, cpu);
	// uint8_t bitConst = GetBitConst(signedEntry.constMask, cpu);
    
    // if (bitConst == 2)
    //     instruction.mnemonic = "ADC";
    
	// DecodeMod(mod, rm, instruction, cpu);

	// instruction.immediate = LoadImmediateSigned(instruction.width, instruction.sign, cpu);
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
	std::vector<Instruction> decodedInstructions = {};
}

/**
 * @brief Main decoding function that dispatches to specific decoders based on instruction category.
 * @param instruction The instruction structure to populate with decoded information.
 * @param entry The instruction table entry containing mnemonic and encoding details.
 * @param cpu The CPU structure; PC is advanced during decoding.
 * @note This function emulates the 8086's instruction decoding pipeline, routing opcodes to appropriate microcode-like handlers.
 */
Instruction Decode(InstructionTableEntry& entry, struct CPU& cpu)
{
	Instruction instruction;
	instruction.mnemonic = entry.mnemonic;

	switch (entry.category)
	{
	case ENCODING_TWO_BYTE_LOGIC:
	{
		DecodeTwoByteLogic(instruction, entry, cpu);
	} break;
	case ENCODING_TWO_BYTE_LOGIC_IMMEDIATE:
	{
		DecodeTwoByteLogicImmediate(instruction, entry, cpu);
	} break;
	case ENCODING_ONE_BYTE_LOGIC_IMMEDIATE:
	{
		DecodeOneByteLogicImmediate(instruction, entry, cpu);
	} break;
	case ENCODING_ONE_BYTE_ACCUMULATOR:
	{
		DecodeAccumulator(instruction, entry, cpu);
	} break;
	case ENCODING_TWO_BYTE_IMMEDIATE_SIGNED:
	{
		DecodeArithmeticTwoByteSigned(instruction, entry, cpu);
	} break;
	}

	return instruction;
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
	std::string asmFile = argv[1];
	struct Program program = LoadProgramIntoMemory(asmFile);

	struct CPU cpu = { 0 };

	while (cpu.IP != program.size)
	{
		uint8_t currentByte = GetInstructionByte(cpu.IP);

		// Get opcode from byte
		std::optional<InstructionTableEntry> opcodeResult = FindInstruction(currentByte);
		if (!opcodeResult)
		{
			std::cerr << std::format("The byte did not map to a valid 8086 instruction::{}\n", std::bitset<8>(currentByte).to_string());
			return 1;
		}

		InstructionTableEntry entry = *opcodeResult;

		// Not sure if we even need to return an instruction? Maybe just for outputing to file? 
		Instruction instruction = Decode(entry, cpu);

		// Write to asm file if in disassemble mode

		// Execute if in simulate mode
		Execute(cpu);
	}


	return 0;
}
