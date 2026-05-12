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
 * TODO: We need to define a register table to look up the human friendly name for these registers. 
 */


/**
 * @brief Retrieve a byte from a prgram in memory at the specified Instruction Pointer. 
 * The instruction pointer is incremented by 1 after the byte is retrieved from memory.
 */
uint8_t GetInstructionByte(uint16_t &ip, bool incrementIp = true) {
	uint8_t byte = Memory[ip];
	if (incrementIp) ip++;
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
	uint8_t width;			// Width of the encoded operands
	uint8_t sign; 			// Sign of the immediate operand if an arithmetic instruction 
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
	EncodingCategory category;
	uint8_t sMask;
	uint8_t wMask;
	uint8_t dMask;
	uint8_t modMask;
	uint8_t rmMask;
	uint8_t regMask;
	uint8_t constMask;
	uint8_t direction;
};

InstructionTableEntry InstructionTable[] = {
	{ .opcode=0x88, .opcodeMask=0xFC, .category=ENCODING_TWO_BYTE_LOGIC, .dMask=0x02, .wMask=0x01, .modMask=0xC0, .regMask=0x38, .rmMask=0x07 }
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
	uint8_t lowByte = GetInstructionByte(cpu.IP);
	uint8_t highByte = GetInstructionByte(cpu.IP);

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
	uint8_t byte = GetInstructionByte(cpu.IP);
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

Mnemonic DecodeMnemonic(uint8_t byte)
{
	switch(byte)
	{
		case 0:
			return ADD;
		case 1:
			return SUB;
		case 2:
			return ADC;
		default:
			return MOV;
	}
}

/**
 * @brief Decodes the MOD field and performs microcode operations for the specific mod encodings as specified in the 8086 manual.
 * @param mod The MOD value (0-3).
 * @param rm The RM value (0-7).
 * @param instruction The instruction structure to update with the mnemonic.
 * @param cpu The CPU structure; PC is advanced as needed.
 * @note This function emulates 8086's microcode for parsing effective addresses, handling displacements and register modes.
 */
Operand InterpretModField(uint8_t mod, uint8_t rm, struct CPU &cpu)
{
	Operand operand = { 0 };
	switch (mod)
	{
	case 0x00: // Memory Mode, No Displacement
	{
		// Special case of MOD that indicates direct memory access with no displacement. 
		if (rm == 0x6)
		{
			operand.directAddress = LoadWordData(cpu);
		}	
		else
		{
			operand = EffectiveAddressCalculation[rm];

		}	
	} break;
	case 0x01: // Memory Mode, 8-bit displacement
	{
		operand = EffectiveAddressCalculation[rm];
		operand.displacement = LoadByteData(cpu);
	} break;
	case 0x02: // Memory Mode, 16-bit displacement
	{
		operand = EffectiveAddressCalculation[rm];
		operand.displacement = LoadWordData(cpu);
	} break;
	case 0x03: // Register Mode, No Displacement
	{
        operand.baseRegister = rm;
	} break;
	}

	return operand;
}

OperandType InterpretRmOperandType(uint8_t mod, uint8_t rm)
{
	switch (mod)
	{
	case 0x00: // Memory Mode, No Displacement
	{
		// Special case of MOD that indicates direct memory access with no displacement. 
		if (rm == 0x6)
			return DIRECT_ADDRESS;
		else
			return EFFECTIVE_ADDRESS_CALC;
	}
	case 0x01: // Memory Mode, 8-bit displacement
	{
		return EFFECTIVE_ADDRESS_CALC_W_DISPLACEMENT;
	} 
	case 0x02: // Memory Mode, 16-bit displacement
	{
		return EFFECTIVE_ADDRESS_CALC_W_DISPLACEMENT;
	} 
	case 0x03: // Register Mode, No Displacement
	{
        return REGISTER;
	} 
	default: // Just to prevent compiler warnings, there has to be a better way to handle this
	{
		return REGISTER;
	}
	}
}

/**
 * When direction == 0, operandA goes into src 
 * When direction == 1, operandB goes into src
 */
void InterpretOperandDirection(Operand operandA, OperandType operandAType, Operand operandB, OperandType operandBType, Instruction &instruction, uint8_t direction)
{
	if (direction == 0) // SRC is in REG field
	{
		instruction.src = operandA;
		instruction.srcType = operandAType;
		instruction.dest = operandB;
		instruction.destType = operandBType;
	}
	else // DEST is in REG field
	{
		instruction.src = operandB;
		instruction.srcType = operandBType;
		instruction.dest = operandA;
		instruction.destType = operandAType;
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
	uint8_t currentByte = GetInstructionByte(cpu.IP);

	// Get width if w bit is present 
	/* CODE REVIEW: There is probably a way to do this without a conditional jump instruction */
	if (logicEntry.wMask != 0)  instruction.width = (currentByte & logicEntry.wMask);

	// Get direction if d bit is present, direction is always in bit 1 if it is present so we can just shift right by 1 to get the value
	uint8_t direction;
	/* CODE REVIEW: There is probably a way to do this without the conditional */
	if (logicEntry.dMask != 0) direction = (currentByte & logicEntry.dMask) >> 1;

	currentByte = GetInstructionByte(cpu.IP);
	Operand regOperand = { 0 };
	OperandType regOperandType;
	/* CODE REVIEW: MIs this necesary? What happens if regMask is 0, what is the type? We may be able to remove this conditional */
	if (logicEntry.regMask != 0) 
	{
		regOperand.baseRegister = DecodeReg(logicEntry.regMask, logicEntry.regShift, currentByte);
		regOperandType = REGISTER;
	}
	
	uint8_t mod = DecodeMod(logicEntry.modMask, currentByte);
	uint8_t rm = DecodeRm(logicEntry.rmMask, currentByte);
	Operand rmOperand = InterpretModField(mod, rm, cpu);
	OperandType rmOperandType = InterpretRmOperandType(mod, rm);

	InterpretOperandDirection(regOperand, regOperandType, rmOperand, rmOperandType, instruction, direction);

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

	uint8_t currentByte = GetInstructionByte(cpu.IP);

	// Get width if w bit is present 
	if (logicImmediateEntry.wMask != 0) instruction.width = (currentByte & logicImmediateEntry.wMask);

	currentByte = GetInstructionByte(cpu.IP);
	uint8_t mod = DecodeMod(logicImmediateEntry.modMask, currentByte);
	uint8_t rm = DecodeRm(logicImmediateEntry.rmMask, currentByte);
	instruction.dest = InterpretModField(mod, rm, cpu);
	instruction.destType = InterpretRmOperandType(mod, rm);

	instruction.src = { 0 };
	instruction.src.immediate = instruction.width == 0 ? LoadByteData(cpu) : LoadWordData(cpu);
	instruction.srcType = IMMEDIATE;
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
	uint8_t currentByte = GetInstructionByte(cpu.IP);
	instruction.width = (currentByte & logicImmediateEntry.wMask) >> logicImmediateEntry.wShift;

	uint8_t reg = (currentByte & logicImmediateEntry.regMask);
	
	instruction.dest = { 0 };
	instruction.dest.baseRegister = reg;
	instruction.destType = REGISTER;

	instruction.src = { 0 };
	instruction.srcType = IMMEDIATE;

	 instruction.src.immediate = instruction.width == 0 ? LoadByteData(cpu) : LoadWordData(cpu);
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
	struct OneByteAccumulatorEntry accumulatorEntry = entry.encoding.threeByteAccumulatorEncoding;
	uint8_t direction = accumulatorEntry.direction;

	uint8_t currentByte = GetInstructionByte(cpu.IP);
	instruction.width = (currentByte & accumulatorEntry.wMask);

	Operand operandA = { 0 }; 
	OperandType operandAType = REGISTER;

	Operand operandB = { 0 };
	OperandType operandBType;
	/* CODE REVIEW: Can we reduce conditionals here? (including ternary) */
    if (accumulatorEntry.hasAddress)
    {
		operandB.directAddress = LoadWordData(cpu);
		operandBType = DIRECT_ADDRESS;
	}    
    else
    {
		operandB.immediate = instruction.width == 0 ? LoadByteData(cpu) : LoadWordData(cpu);
		operandBType = IMMEDIATE;
	}   

	InterpretOperandDirection(operandA, operandAType, operandB, operandBType, instruction, direction);
    
}

void DecodeTwoByteArithmetic(Instruction& instruction, InstructionTableEntry& entry, struct CPU& cpu)
{
	TwoByteImmedSignedEntry signedEntry = entry.encoding.arithmeticTwoByteImmedSignedEntry;

	uint8_t currentByte = GetInstructionByte(cpu.IP);

	uint8_t sign = (currentByte & signedEntry.sMask) >> 1;
	instruction.width = currentByte & signedEntry.wMask;

	// Increment because all the following data is in byte 2
	currentByte = GetInstructionByte(cpu.IP);
	uint8_t mod = DecodeMod(signedEntry.modMask, currentByte);
	uint8_t rm = DecodeRm(signedEntry.rmMask, currentByte);
	uint8_t bitConst = DecodeBitConst(signedEntry.constMask, currentByte);
    
    instruction.mnemonic = DecodeMnemonic(bitConst);
    instruction.dest = InterpretModField(mod, rm, cpu);
	instruction.destType = InterpretRmOperandType(mod, rm);

	instruction.src.immediate = (sign == 0 && instruction.width == 1) ? LoadWordData(cpu) : LoadByteData(cpu);
	instruction.srcType = IMMEDIATE;
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
		DecodeTwoByteArithmetic(instruction, entry, cpu);
	} break;
	}

	return instruction;
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
		uint8_t currentByte = GetInstructionByte(cpu.IP, false);

		// Get opcode from byte
		/* CODE REVIEW: I need to dig deeper into how std::optional works, I feel like this is overkill and probably can be accomplished with simple pointers. This would avoid the dereferencing below. */
		std::optional<InstructionTableEntry> opcodeResult = FindInstruction(currentByte);
		if (!opcodeResult)
		{
			std::cerr << std::format("The byte did not map to a valid 8086 instruction::{}\n", std::bitset<8>(currentByte).to_string());
			return;
		}

		InstructionTableEntry entry = *opcodeResult;

		Instruction instruction = Decode(entry, cpu);
 
		WriteToFile(instruction, file);
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
