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
 * @brief Struct that represents the state of the CPU, including memory and registers
 */
struct CPU {
	uint8_t memory[1024 * 1024]; // 1 MB
	uint16_t PC;
};

enum EncodingCategory {
	ENCODING_TWO_BYTE_LOGIC,
	ENCODING_THREE_BYTE_ACCUMULATOR,
	ENCODING_TWO_BYTE_LOGIC_IMMEDIATE,
	ENCODING_ONE_BYTE_LOGIC_IMMEDIATE,
	ENCODING_ARITHMETIC_TWO_BYTE_IMMEDIATE_SIGNED
};

/**
 * @brief Represents a decoded instructions
 * TODO: This can probably go away at some point and we can just write directly to a buffer or file. Its more for debugging
 */
struct Instruction
{
	uint8_t direction;				// Determines if reg or rm goes first in assembly output
	uint8_t width;					// Width of the register or data 
	uint8_t sign;
	int16_t immediate;				// Immediate value if there is one
	uint16_t address;				// Address if instruction has an address (i.e. MOV AL, [1234h])
	std::string mnemonic;		// Human readable assembly language mnemonic for the instruction
	std::string regMnemonic;	// The human readable name for the register (i.e. AX, BX, etc.)
	std::string rmMnemonic;	// The human redable name for the register stored in R/M when R/M is used to hold register info 
};


/* ====================================================================== */
/* ==============     Intel 8086 Instruction Table    =================== */
/* ====================================================================== */

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

struct ThreeByteAccumulatorEntry
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

struct ArithmeticTwoByteImmedSignedEntry 
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
	const char* mnemonic;
	EncodingCategory category;
	union
	{
		struct TwoByteLogicEntry twoByteLogicEntry;
		struct TwoByteLogicImmediateEntry twoByteLogicImmediateEntry;
		struct ThreeByteAccumulatorEntry threeByteAccumulatorEncoding;
		struct OneByteLogicImmediateEntry oneByteLogicImmediateEncoding;
		struct ArithmeticTwoByteImmedSignedEntry arithmeticTwoByteImmedSignedEntry;

	} encoding;
};

// TODO (joe): Maybe remove strings from this table and create a mapping table of opcode -> mnemonic
#define InstructionTableEntries \
	X(0x88, 0xFC, "MOV", ENCODING_TWO_BYTE_LOGIC, 0x02,  0x01, 0xC0, 0x38, 0x3, 0x07), \
	X(0xC6, 0xFE, "MOV", ENCODING_TWO_BYTE_LOGIC_IMMEDIATE, 0x01, 0xC0, 0x07), \
	X(0xB0, 0xF0, "MOV", ENCODING_ONE_BYTE_LOGIC_IMMEDIATE, 0x08, 0x03, 0x07), \
	X(0xA0, 0xFE, "MOV", ENCODING_THREE_BYTE_ACCUMULATOR, 0x01, 0x01, true), \
	X(0xA2, 0xFE, "MOV", ENCODING_THREE_BYTE_ACCUMULATOR, 0x00, 0x01, true), \
	X(0x00, 0xFC, "ADD", ENCODING_TWO_BYTE_LOGIC, 0x02,  0x01, 0xC0, 0x38, 0x3, 0x07), \
	X(0x80, 0xFC, "ADD", ENCODING_ARITHMETIC_TWO_BYTE_IMMEDIATE_SIGNED,  0x02, 0x01, 0xC0, 0x07, 0x38, 0x01), \
    X(0x04, 0xFE, "ADD", ENCODING_THREE_BYTE_ACCUMULATOR, 0x01, 0x01, false), \
	X(0x14, 0xFE, "ADC", ENCODING_THREE_BYTE_ACCUMULATOR, 0x01, 0x01, false), \
	X(0x10, 0xFC, "ADC", ENCODING_TWO_BYTE_LOGIC, 0x02,  0x01, 0xC0, 0x38, 0x3, 0x07), \
	X(0x28, 0xFC, "SUB", ENCODING_TWO_BYTE_LOGIC, 0x02,  0x01, 0xC0, 0x38, 0x3, 0x07), \
	X(0x2C, 0xFE, "SUB", ENCODING_THREE_BYTE_ACCUMULATOR, 0x01, 0x01, false) \

#define X(opcode, opcodeMask, mnemonic, category, ...) { opcode, opcodeMask, mnemonic, category, { __VA_ARGS__ } }
std::vector<InstructionTableEntry> instructionTable = {
	InstructionTableEntries
};
#undef X

/* ====================================================================== */
/* ==============     Register (1 & 2 byte) Table     =================== */
/* ====================================================================== */

#define RegistersWide8086 \
	X(0b00000000, "AX") \
	X(0b00000001, "CX") \
	X(0b00000010, "DX") \
	X(0b00000011, "BX") \
	X(0b00000100, "SP") \
	X(0b00000101, "BP") \
	X(0b00000110, "SI") \
	X(0b00000111, "DI") \

#define Registers8086 \
	X(0b00000000, "AL") \
	X(0b00000001, "CL") \
	X(0b00000010, "DL") \
	X(0b00000011, "BL") \
	X(0b00000100, "AH") \
	X(0b00000101, "CH") \
	X(0b00000110, "DH") \
	X(0b00000111, "BH") \

#define X(name, code) {name, code},
std::unordered_map<uint8_t, const char*> registerWideTable = {
	RegistersWide8086
};

std::unordered_map<uint8_t, const char*> registerTable = {
	Registers8086
};
#undef X

/**
 * @brief Get register mnemonic from register code and width. This is used to decode the reg field in the mod byte when it is used to hold register information.
 * @param reg 
 * @param width 
 * @return register mnemonic
 */
const char* GetRegister(uint8_t reg, uint8_t width)
{
	// Get 16 bit register
	if (width)
		return registerWideTable.at(reg);

	// Get 8 bit register
	return registerTable.at(reg);
}


/* ====================================================================== */
/* ============== Effective Address Calculation Table =================== */

#define ModCalculations \
	X(/* R/M = 000*/ 0b00000000, "BX + SI") \
	X(/* R/M = 001*/ 0b00000001, "BX + DI") \
	X(/* R/M = 010*/ 0b00000010, "BP + SI") \
	X(/* R/M = 011*/ 0b00000011, "BP + DI") \
	X(/* R/M = 100*/ 0b00000100, "SI") \
	X(/* R/M = 101*/ 0b00000101, "DI") \
	X(/* R/M = 110*/ 0b00000110, "[]") \
	X(/* R/M = 111*/ 0b00000111, "BX") \

#define X(code, calcLiteral) { code, calcLiteral },
std::unordered_map<uint8_t, const char*> modEffectiveAddressTable = {
	ModCalculations
};
#undef X


/* ====================================================================== */
/* ==============      Table operation functions      =================== */
/* ====================================================================== */


// TODO: This might be better off returning an instruction entry instead of an index
/**
 * @brief Search instruction table for instruction that matches the opcode in the byte. 
 * @param byte The byte containing the opcode to search for in the instruction table.
 * @return Returns index of instruction in instruction table if found, otherwise returns -1.
 */
int FindInstruction(uint8_t byte)
{
	for (int i = 0; i < instructionTable.size(); i++)
	{
		// Get opcode from byte
		InstructionTableEntry inst = instructionTable.at(i);
		uint8_t b = byte & inst.opcodeMask;

		// If byte matches opcode, break out and return index of instruction
		if (b == inst.opcode)
		{
			return i;
		}
	}

	// Byte doesn't match any instructions
	return -1;
}


struct CPU cpu = { 0 };

void writeToFile(std::vector<std::string> instructions)
{
    std::ofstream asmFile;
    asmFile.open("result.asm");

    asmFile << "bits 16\n\n";

    for (auto inst : instructions)
    {
        asmFile << inst;
    }
    
    asmFile << "hlt"; 

    asmFile.close();
}

/**
 * @brief Looks up an opcode byte in the instruction table.
 * @param byte The opcode byte to decode.
 * @return An optional containing the instruction table entry if found, or std::nullopt if invalid.
 * @note This function performs the initial opcode lookup in the 8086 decoding process, similar to the control store access.
 */
std::optional<InstructionTableEntry> decodeOpcode(const uint8_t byte)
{
	int instructionEntry = FindInstruction(byte);

	if (instructionEntry < 0) return std::nullopt;

	return std::make_optional(instructionTable.at(instructionEntry));
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
	uint8_t lowByte = cpu.memory[cpu.PC];
	uint16_t highByte = cpu.memory[++cpu.PC];
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
	int8_t lowByte = static_cast<int8_t>(cpu.memory[cpu.PC]);
    int16_t data = static_cast<int16_t>(lowByte);
	return data;
}

/**
 * @brief Loads an immediate value from memory based on the specified width.
 * @param width 0 for 8-bit, 1 for 16-bit.
 * @param cpu The CPU structure containing memory and PC.
 * @return The loaded immediate value as a 16-bit unsigned integer.
 * @note Increments PC. In 8086, immediates follow opcodes and are used directly in instructions like MOV immediate.
 */
uint16_t LoadImmediate(uint8_t width, struct CPU &cpu)
{
	cpu.PC++;

	if (width) return LoadWordData(cpu);

	return LoadByteData(cpu);
}

uint16_t LoadImmediateSigned(uint8_t width, uint8_t sign, struct CPU &cpu)
{
	cpu.PC++;

	uint8_t size = width | sign;

	switch(size)
	{
		case 0:
		{
			return LoadByteData(cpu);
		} break;
		case 1:
		{
			return LoadWordData(cpu);
		} break;
		case 2:
		{
			// Read 1 byte, sign extend to 16
		} break;
		case 3:
		{
            return LoadByteData(cpu);
		} break;
	};
}

/**
 * @brief Extracts the MOD field from the current opcode byte.
 * @param modMask Bitmask to isolate the MOD bits (typically 0xC0).
 * @param cpu The CPU structure.
 * @return The 2-bit MOD value (0-3), indicating addressing mode.
 * @note In 8086, MOD (bits 7-6 of MOD-RM byte) specifies memory/register mode: 00=mem no disp, 01=mem 8-bit disp, 10=mem 16-bit disp, 11=register.
 */
uint8_t GetMod(uint8_t modMask, struct CPU& cpu)
{
	return (cpu.memory[cpu.PC] & modMask) >> 6;
}

/**
 * @brief Extracts the RM field from the current opcode byte.
 * @param rmMask Bitmask to isolate the RM bits (typically 0x07).
 * @param cpu The CPU structure.
 * @return The 3-bit RM value (0-7), indicating register or effective address base.
 * @note In 8086, RM (bits 2-0 of MOD-RM byte) selects the register or base for addressing (e.g., BX+SI).
 */
uint8_t GetRm(uint8_t rmMask, struct CPU& cpu)
{
	return (cpu.memory[cpu.PC] & rmMask);
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
uint8_t GetReg(uint8_t regMask, uint8_t regShift, Instruction& instruction, struct CPU& cpu)
{
	return (cpu.memory[cpu.PC] & regMask) >> regShift;
}

/**
 * @brief Extracts a constant bit field from the current opcode byte.
 * @param constMask Bitmask to isolate the constant bits.
 * @param cpu The CPU structure.
 * @return The extracted bit value, shifted to the least significant position.
 * @note In 8086 arithmetic instructions, this extracts specific control bits from the opcode.
 */
uint8_t GetBitConst(uint8_t constMask, struct CPU& cpu)
{
	return (cpu.memory[cpu.PC] & constMask) >> 3;
}

/**
 * @brief Decodes the MOD-RM byte to determine the addressing mode and formats the operand mnemonic.
 * @param mod The MOD value (0-3).
 * @param rm The RM value (0-7).
 * @param instruction The instruction structure to update with the mnemonic.
 * @param cpu The CPU structure; PC is advanced as needed.
 * @note This function emulates 8086's microcode for parsing effective addresses, handling displacements and register modes.
 */
void DecodeMod(uint8_t mod, uint8_t rm, Instruction& instruction, struct CPU &cpu)
{
	switch (mod)
	{
	case 0x00:
	{
		// Special case of MOD that indicates direct memory access with no displacement. 
		if (rm == 0x6)
		{
			cpu.PC++;
			instruction.rmMnemonic = std::format("[{}]", LoadWordData(cpu));
		}
		else
			instruction.rmMnemonic = std::format("[{}]", modEffectiveAddressTable.at(rm));
	} break;
	case 0x01:
	{
		// Need to sign extend properly to cast to signed int16, otherwise we will get incorrect values for negative displacements.
		int16_t data = static_cast<int16_t>(LoadImmediate(0, cpu));
		if (data < 0)
		{
			data = ~data + 1; // get the positive value of the negative displacement for printing
			instruction.rmMnemonic = std::format("[{} - {}]", modEffectiveAddressTable.at(rm), data);
		}
		else
			instruction.rmMnemonic = std::format("[{} + {}]", modEffectiveAddressTable.at(rm), data);
	} break;
	case 0x02:
	{
		int16_t data = static_cast<int16_t>(LoadImmediate(1, cpu));
		if (data < 0)
		{
			data = ~data + 1; // get the positive value of the negative displacement for printing
			instruction.rmMnemonic = std::format("[{} - {}]", modEffectiveAddressTable.at(rm), data);
		}
		else
			instruction.rmMnemonic = std::format("[{} + {}]", modEffectiveAddressTable.at(rm), data);
	} break;
	case 0x03:
	{
		instruction.rmMnemonic = std::format("{}", GetRegister(rm, instruction.width));
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

	// Get width if w bit is present 
	if (logicEntry.wMask != 0)  instruction.width = (cpu.memory[cpu.PC] & logicEntry.wMask);

	// Get direction if d bit is present, direction is always in bit 1 if it is present so we can just shift right by 1 to get the value
	if (logicEntry.dMask != 0) instruction.direction = (cpu.memory[cpu.PC] & logicEntry.dMask) >> 1;

	cpu.PC++;
	if (logicEntry.regMask != 0)
	{
		uint8_t reg = GetReg(logicEntry.regMask, logicEntry.regShift, instruction, cpu);
		instruction.regMnemonic = std::format("{}", GetRegister(reg, instruction.width));
	}
	uint8_t mod = GetMod(logicEntry.modMask, cpu);
	uint8_t rm = GetRm(logicEntry.rmMask, cpu);
	DecodeMod(mod, rm, instruction, cpu);
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

	// Get width if w bit is present 
	if (logicImmediateEntry.wMask != 0) instruction.width = (cpu.memory[cpu.PC] & logicImmediateEntry.wMask);

	cpu.PC++;
	uint8_t mod = GetMod(logicImmediateEntry.modMask, cpu);
	uint8_t rm = GetRm(logicImmediateEntry.rmMask, cpu);
	DecodeMod(mod, rm, instruction, cpu);
	instruction.immediate = LoadImmediate(instruction.width, cpu);
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
	instruction.width = (cpu.memory[cpu.PC] & logicImmediateEntry.wMask) >> logicImmediateEntry.wShift;

	uint8_t reg = (cpu.memory[cpu.PC] & logicImmediateEntry.regMask);
	instruction.regMnemonic = std::format("{}", GetRegister(reg, instruction.width));

	instruction.immediate = LoadImmediate(instruction.width, cpu);
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
	struct ThreeByteAccumulatorEntry accumulatorEntry = entry.encoding.threeByteAccumulatorEncoding;
	instruction.direction = accumulatorEntry.direction;
	instruction.width = (cpu.memory[cpu.PC] & accumulatorEntry.wMask);
    if (accumulatorEntry.hasAddress)
        instruction.address = LoadImmediate(1, cpu);
    else
        
        instruction.immediate = LoadImmediate(instruction.width, cpu);
    
	instruction.regMnemonic = std::format("{}", GetRegister(0x00, instruction.width));
    
}

void DecodeArithmeticTwoByteSigned(Instruction& instruction, InstructionTableEntry& entry, struct CPU& cpu)
{
	ArithmeticTwoByteImmedSignedEntry signedEntry = entry.encoding.arithmeticTwoByteImmedSignedEntry;

	uint8_t s = (cpu.memory[cpu.PC] & signedEntry.sMask) >> 1;
	instruction.width = cpu.memory[cpu.PC] & signedEntry.wMask;
	instruction.sign = cpu.memory[cpu.PC] & signedEntry.sMask;

	// Increment because all the following data is in byte 2
	cpu.PC++;
	uint8_t mod = GetMod(signedEntry.modMask, cpu);
	uint8_t rm = GetRm(signedEntry.rmMask, cpu);
	uint8_t bitConst = GetBitConst(signedEntry.constMask, cpu);
    
    if (bitConst == 2)
        instruction.mnemonic = "ADC";
    
	DecodeMod(mod, rm, instruction, cpu);

	instruction.immediate = LoadImmediateSigned(instruction.width, instruction.sign, cpu);
}

/**
 * @brief Main decoding function that dispatches to specific decoders based on instruction category.
 * @param instruction The instruction structure to populate with decoded information.
 * @param entry The instruction table entry containing mnemonic and encoding details.
 * @param cpu The CPU structure; PC is advanced during decoding.
 * @note This function emulates the 8086's instruction decoding pipeline, routing opcodes to appropriate microcode-like handlers.
 */
void Decode(Instruction& instruction, InstructionTableEntry& entry, struct CPU& cpu)
{
	instruction.mnemonic = std::format("{}", entry.mnemonic);

	switch (entry.category)
	{
	case ENCODING_TWO_BYTE_LOGIC:
	{
		DecodeTwoByteLogic(instruction, entry, cpu);
	}break;
	case ENCODING_TWO_BYTE_LOGIC_IMMEDIATE:
	{
		DecodeTwoByteLogicImmediate(instruction, entry, cpu);
	}break;
	case ENCODING_ONE_BYTE_LOGIC_IMMEDIATE:
	{
		DecodeOneByteLogicImmediate(instruction, entry, cpu);
	} break;
	case ENCODING_THREE_BYTE_ACCUMULATOR:
	{
		DecodeAccumulator(instruction, entry, cpu);
	}break;
	case ENCODING_ARITHMETIC_TWO_BYTE_IMMEDIATE_SIGNED:
	{
		DecodeArithmeticTwoByteSigned(instruction, entry, cpu);
	} break;
	}
}

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
	while (cpu.memory[cpu.PC] != 0xF4)
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
