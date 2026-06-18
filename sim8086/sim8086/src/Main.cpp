// sim8086.cpp : Defines the entry point for the application.

#include <cstdint>
#include <fstream>
#include <iostream>
#include <cstdio>

/*
    TODO: Left off determining the best way to map register Index/offset to register names for pretty printing the assembly instructions. We probably just need an array of some kind with the string constants 
*/

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

enum RegisterIndex {

    Register_a,
    Register_b,
    Register_c,
    Register_d,
    Register_sp,
    Register_bp,
    Register_si,
    Register_di,

    Register_count
};

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

struct RegisterAccess {
    uint8_t index;      // index of the register in the 8086 manual. For example, register AX/AL is 000 while register CX/CL is 001
    uint8_t offset;     // offset in the register, 0 - low bits, 1 - high bits, 2 - full 16 bits (no offset)
};

enum ModCategory: uint8_t 
{
	Memory_mode_no_disp,
	Memory_mode_8_bit_disp,
	Memory_mode_16_bit_disp,
	Register_mode,

	Mod_category_count
};

enum EffectiveAddressCalculation: uint8_t 
{
	Effective_addr_direct_address,

	Effective_addr_bx_si,
	Effective_addr_bx_di,
	Effective_addr_bp_si,
	Effective_addr_bp_di,
	Effective_addr_si,
	Effective_addr_di,
	Effective_addr_bx,

	Effective_addr_count
};

struct EffectiveAddrExpression
{
    EffectiveAddressCalculation calculationType;
	RegisterAccess base;
	RegisterAccess index;
	int16_t displacement;
};

void DecodeEffectiveAddrExpression(uint8_t mod, uint8_t rm, EffectiveAddrExpression &expression) 
{
    switch(rm)
    {
        case 0b000:
        {
            expression.base.index = Register_b;
            expression.base.offset = 2;

            expression.index.index = Register_si;
            expression.index.offset = 2;
        } break;
        default:
        {

        }
    }
}

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

struct Entry {
	const char* mnemonic;
	Bits bits[16];	// bits[0] = Opcode bits
};

Entry InstructionTable[] = {
	#include "InstructionTable.inl"
};

/* Operation Definitions */

enum Operation: uint8_t {

    #define INST(mnemonic, ...) Op_##mnemonic
    #define INST_ALT(...)
	#include "InstructionTable.inl"

	Op_count
};

const char* Mnemonics[] = {
    #define INST(mnemonic, ...) #mnemonic
    #define INST_LAT(...)
    #include "InstructionTable.inl"
};

const char* RegisterNames[Register_count][3] = {
    {"AL", "AH", "AX"},
	{"BL", "BH", "BX"},
    {"CL", "CH", "CX"},
    {"DL", "DH", "DX"},
	{"", "", "SP"},
	{"", "", "BP"},
	{"", "", "SI"},
	{"", "", "DI"}
    
};

// Constants to represent different parts of register access
#define LO_BITS 0
#define HI_BITS 1
#define FULL_BITS 2


void DecodeRegister(uint8_t reg, uint8_t w, RegisterAccess &regAccess)
{
    switch(reg)
    {
        case 0b000:
        {
            regAccess.index = Register_a;
            regAccess.offset = (w == 0) ? LO_BITS : FULL_BITS;
        } break; 
        case 0b001:
        {
        	regAccess.index = Register_c;
			regAccess.offset = (w == 0) ? LO_BITS : FULL_BITS;	
        } break;
        case 0b010:
        {
        	regAccess.index = Register_d;
			regAccess.offset = (w == 0) ? LO_BITS : FULL_BITS;	
        } break;  
        case 0b011:
        {
        	regAccess.index = Register_b;
			regAccess.offset = (w == 0) ? LO_BITS : FULL_BITS;	
        } break;  
        case 0b100:
        {
        	regAccess.index = w == 0 ? Register_a : Register_sp;
			regAccess.offset = (w == 0) ? HI_BITS : FULL_BITS;	
        } break;
        case 0b101:
        {
        	regAccess.index = w == 0 ? Register_c : Register_bp;
			regAccess.offset = (w == 0) ? HI_BITS : FULL_BITS;	
        } break;
        case 0b110:
        {
        	regAccess.index = w == 0 ? Register_a : Register_si;
			regAccess.offset = (w == 0) ? HI_BITS : FULL_BITS;	
        } break;
        case 0b111:
        {
        	regAccess.index = w == 0 ? Register_a : Register_di;
			regAccess.offset = (w == 0) ? HI_BITS : FULL_BITS;	
        } break;
    }
}


enum OperandType {
	OpType_register,
	OpType_effectiveAddrCalc,
	OpType_immediate,

	OpType_count
};

struct Operand {
	OperandType type;
	union {
		RegisterAccess reg;
		EffectiveAddrExpression expression;
		int16_t immediate;
	};
};

struct Instruction {
	Operation op;
	uint8_t w;
	uint8_t d;
	Operand operands[2];
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

void PrintOperand(Operand op)
{
	switch(op.type)
	{
		case OpType_register:
		{
			const char* name = RegisterNames[op.reg.index][op.reg.offset];
			printf("%s", name);		
		} break;
        case OpType_effectiveAddrCalc:
        {
            const char* base = RegisterNames[op.expression.base.index][op.expression.base.offset];
            const char* index = RegisterNames[op.expression.index.index][op.expression.index.offset];
            printf("[%s + %s]", base, index);
        } break;
		default:
		{

		}
	}
}

void WriteToFile()
{
	/** TODO: Write to file */
}

void WriteToConsole(Instruction inst) 
{
	// Print mnemonic/operation 
	printf("%s ", Mnemonics[inst.op]);

	// Print dest operand 
	PrintOperand(inst.operands[0]);
	
	printf(", ");
	
	// Print src operand 
	PrintOperand(inst.operands[1]);
	
	printf("\n");
}


/**
 * Used to determine if all bits in an instruction have been decoded. 
 * If all fields are 0, then its an uninitialized struct and is the end
 * of the bits array. 
 */
bool IsBitsDefined(Bits bits)
{
	return !(bits.field == Literal
		&& bits.count == 0
		&& bits.shift == 0
		&& bits.value == 0);
}

void Execute(struct CPU &cpu)
{
	// TODO....
}

void InterpretModRm(CPU &cpu, uint8_t mod, uint8_t rm, uint8_t w,  Operand &operand)
{
	switch(mod)
	{
		case Memory_mode_no_disp:
		{
            EffectiveAddrExpression exp = {};
            DecodeEffectiveAddrExpression(mod, rm, exp);
            operand.type = OpType_effectiveAddrCalc;
            operand.expression = exp;

		} break;
		case Memory_mode_8_bit_disp:
		{

		} break;
		case Memory_mode_16_bit_disp:
		{

		} break;
		case Register_mode:
		{
			operand.type = OpType_register;
			operand.reg = {};
			DecodeRegister(rm, w, operand.reg);	
		} break;
	}
}

Instruction Decode(CPU &cpu, Entry entry, uint8_t currentByte)
{
	Instruction inst = {};
	uint8_t byte = currentByte;

	// Instruction opcode matched, begin decode
	uint8_t bitsIndex = 1;
	uint8_t usedBits = entry.bits[0].count;
	uint8_t decodedBits[Field_count];
	uint8_t decodedFields[Field_count];

	// TODO: Need something here to identify if an error occurred and bits could not be parsed
	while (IsBitsDefined(entry.bits[bitsIndex]))
	{

		Bits bit = entry.bits[bitsIndex];
		uint8_t result = (byte >> bit.shift) & bit.value;
		decodedBits[bit.field] = result;
		decodedFields[bit.field] = 1;
		bitsIndex++;
		usedBits += bit.count;

		if (usedBits >= 8 && IsBitsDefined(entry.bits[bitsIndex + 1])) {
			byte = GetNextByte(cpu.IP);
			usedBits = 0;
		}
	
	}


	inst.w = decodedBits[W_bit];
	inst.d = decodedBits[D_bit];

	if (decodedFields[Mod_bit] && decodedFields[Rm_bit])
	{
		Operand op = {};
		InterpretModRm(cpu, decodedBits[Mod_bit], decodedBits[Rm_bit], inst.w, op);
		inst.operands[inst.d] = op;
	}

	if (decodedFields[Reg_bit])
	{
		RegisterAccess a = {};
		DecodeRegister(decodedBits[Reg_bit], inst.w, a);
		inst.operands[!inst.d] = {
			.type = OpType_register,
			.reg = a		
		};
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

		Entry entry = {};
        // Search Instruction table for matching instruction 
        for (int i = 0; i < ArrayCount(InstructionTable); i++)
        {
            entry = InstructionTable[i];

			// Break out of loop if matching opcode is found
            if (entry.bits[0].value == (currentByte >> entry.bits[0].shift))
			{
				break;
			}

        }

		Instruction inst = Decode(cpu, entry, currentByte);
		WriteToConsole(inst);
    }
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
