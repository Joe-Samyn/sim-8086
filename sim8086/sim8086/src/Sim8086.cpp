
#include <cstdint>
#include <fstream>
#include <iostream>
#include <cstdio>


/**
 * Bit Stream Approach
 * -------------------
 * 
 */

#define ArrayCount(array) sizeof(array)/sizeof(array[0])
#define LO_BITS 0
#define HI_BITS 1
#define FULL_BITS 2
#define TRUE 1
#define FALSE 0
#define SRC 0
#define DEST 1
#define OP_EXTENSION_MASK 0b111

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

void DecrementIP(CPU &cpu)
{
    cpu.IP--;
}

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
    return Memory[ip - 1];
}

/**
 * Get the next word (2 bytes) from memory. Increments the IP by 2).
 *
 */
uint16_t GetNextWord(uint16_t &ip)
{
    uint16_t lo = Memory[ip++];
    uint16_t hi = Memory[ip++]; 
    hi = (hi << 8);
    hi = (hi | lo);
    return hi;
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
    Op,
    OpExtension,
    W_bit,
    D_bit,
    Reg_bit,
    Rm_bit,
    Mod_bit,
    Imm_bit,
    Addr_bit,
    Const_bit,
    S_bit,

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
    Effective_addr_bp,

    Effective_addr_count
};

struct EffectiveAddrExpression
{
    EffectiveAddressCalculation calculationType;
    RegisterAccess base;
    RegisterAccess index;
    uint8_t hasDisplacement;
    int16_t displacement;
};

void DecodeEffectiveAddrExpression(uint8_t mod, uint8_t rm, EffectiveAddrExpression &expression, CPU &cpu) 
{
    switch(rm)
    {
        case 0b000:
            {
                expression.calculationType = Effective_addr_bx_si;
                expression.base.index = Register_b;
                expression.base.offset = FULL_BITS ;

                expression.index.index = Register_si;
                expression.index.offset = FULL_BITS;
            } break;
        case 0b001:
            {
                expression.calculationType = Effective_addr_bx_di;
                expression.base.index = Register_b;
                expression.base.offset = FULL_BITS;

                expression.index.index = Register_di;
                expression.index.offset = FULL_BITS;
            } break;
        case 0b010:
            {
                expression.calculationType = Effective_addr_bp_si;
                expression.base.index = Register_bp;
                expression.base.offset = FULL_BITS;

                expression.index.index = Register_si;
                expression.index.offset = FULL_BITS;
            } break;
        case 0b011:
            {
                expression.calculationType = Effective_addr_bp_di;
                expression.base.index = Register_bp;
                expression.base.offset = FULL_BITS;

                expression.index.index = Register_di;
                expression.index.offset = FULL_BITS;
            } break;
        case 0b100:
            {
                expression.calculationType = Effective_addr_si;
                expression.base.index = Register_si;
                expression.base.offset = FULL_BITS;
            } break;
        case 0b101:
            {
                expression.calculationType = Effective_addr_di;
                expression.base.index = Register_di;
                expression.base.offset = FULL_BITS;
            } break;
        case 0b110:
            {
                if (mod == Memory_mode_no_disp)
                {
                    expression.calculationType = Effective_addr_direct_address; 
                    expression.displacement = (int16_t)GetNextWord(cpu.IP);
                }
                else
                {
                    expression.calculationType = Effective_addr_bp;
                    expression.base.index = Register_bp;
                    expression.base.offset = FULL_BITS;
                }
            }break;
        case 0b111:
            {
                expression.calculationType = Effective_addr_bx;
                expression.base.index = Register_b;
                expression.base.offset = FULL_BITS;
            } break; 
    }
}

/* Operation Definitions */

enum Operation: uint8_t {
    None,
#define INST(mnemonic, ...) Op_##mnemonic,
#define INST_ALT(...)
#include "InstructionTable.inl"
#undef INST
#undef INST_ALT
    Op_count
};

const char* Mnemonics[] = {
    "none",
#define INST(mnemonic, ...) #mnemonic,
#define INST_ALT(...)
#include "InstructionTable.inl"
#undef INST
#undef INST_ALT
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

struct Entry {
    Operation mnemonic;
    Bits bits[Field::Field_count];	// bits[0] = Opcode bits TODO: Need to get rid of 16 and use proper constant like Field::Field_count 
};

Entry InstructionTable[] = {
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
                if (w == 0)
                {
                    regAccess.index = Register_a;
                    regAccess.offset = HI_BITS;
                }
                else
                {
                    regAccess.index = Register_sp;
                    regAccess.offset = FULL_BITS;
                }
            } break;
        case 0b101:
            {
                if (w == 0)
                {
                    regAccess.index = Register_c;
                    regAccess.offset = HI_BITS;
                }
                else
                {
                    regAccess.index = Register_bp;
                    regAccess.offset = FULL_BITS;
                }
            } break;
        case 0b110:
            {
                if (w == 0)
                {
                    regAccess.index = Register_d;
                    regAccess.offset = HI_BITS;
                }
                else
                {
                    regAccess.index = Register_si;
                    regAccess.offset = FULL_BITS;
                }
            } break;
        case 0b111:
            {
                if (w == 0)
                {
                    regAccess.index = Register_b;
                    regAccess.offset = HI_BITS;
                }
                else
                {
                    regAccess.index = Register_di;
                    regAccess.offset = FULL_BITS;
                }
            } break;
    }
}


enum OperandType {
    OpType_none, 

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

void PrintEffectiveAddressExpression(Operand op)
{
    switch(op.expression.calculationType)
    {
        case Effective_addr_direct_address:
            {
                printf("[%d]", op.expression.displacement); 
            } break;
        case Effective_addr_bx_si:
        case Effective_addr_bx_di:
        case Effective_addr_bp_si:
        case Effective_addr_bp_di:
            {
                const char* base = RegisterNames[op.expression.base.index][op.expression.base.offset];
                const char* index = RegisterNames[op.expression.index.index][op.expression.index.offset];
                if (op.expression.hasDisplacement == FALSE)
                {
                    printf("[%s + %s]", base, index);
                }
                else
                {   
                    if (op.expression.displacement < 0)
                    {
                        printf("[%s + %s - %d]", base, index, -op.expression.displacement);
                    }
                    else
                    {
                        printf("[%s + %s + %d]", base, index, op.expression.displacement);
                    }
                }
            } break;
        case Effective_addr_si:
        case Effective_addr_di:
        case Effective_addr_bx:
        case Effective_addr_bp:
            {
                const char* base = RegisterNames[op.expression.base.index][op.expression.base.offset];
                if (op.expression.displacement == 0)
                {
                    printf("[%s]", base);
                }
                else
                {
                    if (op.expression.displacement < 0)
                    {
                        printf("[%s - %d]", base, -op.expression.displacement);
                    }
                    else 
                    {
                        printf("[%s + %d]", base, op.expression.displacement);
                    }
                }
            } break;
    }
}

void PrintOperand(Operand op)
{
    switch(op.type)
    {
        case OpType_none:
        {
            return;
        } break;
        case OpType_register:
            {
                const char* name = RegisterNames[op.reg.index][op.reg.offset];
                printf("%s", name);		
            } break;
        case OpType_effectiveAddrCalc:
            {
                PrintEffectiveAddressExpression(op);
            } break;
        case OpType_immediate:
        {
            printf("%d", op.immediate);
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

    // If either operand type is immediate, we should print size 
    if ((inst.operands[SRC].type == OpType_immediate || inst.operands[SRC].type == OpType_none) && inst.operands[DEST].type == OpType_effectiveAddrCalc)
    {
        printf("%s ", inst.w == 0 ? "byte" : "word");
    }

    // Print dest operand 
    PrintOperand(inst.operands[1]);

    if (inst.operands[0].type != OpType_none)
    {
        printf(", ");
    }   

    // Print src operand 
    PrintOperand(inst.operands[0]);

    printf("\n");
}


/**
 * Used to determine if all bits in an instruction have been decoded. 
 * If all fields are 0, then its an uninitialized struct and is the end
 * of the bits array. 
 */
// Does this get inlined and become a preprocessor directive? 
bool IsBitsDefined(Bits bits)
{
    return !(bits.field == Op
            && bits.count == 0
            && bits.shift == 0
            && bits.value == 0);
}

void Execute(struct CPU &cpu)
{
    // TODO....
}

#define DIRECT_ADDRESS 0b110
void InterpretModRm(CPU &cpu, uint8_t mod, uint8_t rm, uint8_t w,  Operand &operand)
{
    switch(mod)
    {
        case Memory_mode_no_disp:
            {
                operand.type = OpType_effectiveAddrCalc;
                EffectiveAddrExpression exp = {};
                DecodeEffectiveAddrExpression(mod, rm, exp, cpu);
                exp.hasDisplacement = FALSE;
                operand.expression = exp;
            } break;
        case Memory_mode_8_bit_disp:
            {
                operand.type = OpType_effectiveAddrCalc;
                EffectiveAddrExpression exp = {};
                DecodeEffectiveAddrExpression(mod, rm, exp, cpu);
                int8_t disp  = (int8_t)GetNextByte(cpu.IP);
                exp.displacement = (int16_t)disp;
                exp.hasDisplacement = TRUE;
                operand.expression = exp;
            } break;
        case Memory_mode_16_bit_disp:
            {
                operand.type = OpType_effectiveAddrCalc;
                EffectiveAddrExpression exp = {};
                DecodeEffectiveAddrExpression(mod, rm, exp, cpu);
                exp.displacement = (int16_t)GetNextWord(cpu.IP);
                exp.hasDisplacement = TRUE;
                operand.expression = exp;
            } break;
        case Register_mode:
            {
                operand.type = OpType_register;
                operand.reg = {};
                DecodeRegister(rm, w, operand.reg);	
            } break;
    }
}

// TODO: Is Entry small enough to be passed via stack? I believe its 64 bytes that can be passed via stack without 
// any cost overhead. We just want to make sure that its not being passed on heap or via pointer 
Instruction Decode(CPU &cpu, Entry entry)
{
    uint8_t byte = GetCurrentByte(cpu.IP);

    uint8_t bitsIndex = 1;
    uint8_t usedBits = entry.bits[0].count;
    uint8_t decodedBits[Field_count] = { 0 };
    uint8_t hasFields[Field_count] = { 0 };
    
    while(IsBitsDefined(entry.bits[bitsIndex]))
    {  

        Bits bit = entry.bits[bitsIndex];
        hasFields[bit.field] = TRUE;
        uint8_t result;

        // Note: If extension doesn't match, no need to continue with any other logic. Just return an
        // Look for next matching instruction.
        if (bit.field == OpExtension)
        {
            result = (byte >> bit.shift) & OP_EXTENSION_MASK;
            if (result != bit.value)
            {
                DecrementIP(cpu);
                return {};
            }    
        }

        // Checking for constant bits 
        if (bit.count == 0)
        {
            // Get literal constant 
            result = bit.value;
        }
        else
        {
            if (usedBits >= 8)
            {
                byte = GetNextByte(cpu.IP);
                usedBits = 0;
            }
            
            result = (byte >> bit.shift) & bit.value;
            
        }
        
        decodedBits[bit.field] = result;
        usedBits += bit.count;
        bitsIndex++;
        
    }

    uint8_t hasD = hasFields[D_bit];
    uint8_t hasS = hasFields[S_bit];
    uint8_t hasMod = hasFields[Mod_bit];
    uint8_t hasRm = hasFields[Rm_bit];
    uint8_t hasReg = hasFields[Reg_bit];
    uint8_t hasImm = hasFields[Imm_bit];
    uint8_t hasAddr = hasFields[Addr_bit];

    uint8_t d = decodedBits[D_bit];
    uint8_t w = decodedBits[W_bit];
    uint8_t s = decodedBits[S_bit];
    uint8_t mod = decodedBits[Mod_bit];
    uint8_t rm = decodedBits[Rm_bit];
    uint8_t reg = decodedBits[Reg_bit];
    
    Instruction inst = {};
    inst.op = entry.mnemonic;
    inst.d = d;
    inst.w = w;

    if (hasMod)
    {
        Operand op = {};
        InterpretModRm(cpu, mod, rm, w, op);
        inst.operands[!d] = op;
    }

    if (hasReg)
    {
        RegisterAccess a = {};
        DecodeRegister(reg, w, a);
        Operand op = {
            .type = OpType_register,
            .reg = a		
        };

        if (hasD)
            inst.operands[d] = op;
        else
            inst.operands[DEST] = op;
    }

    if (hasImm)
    {
        Operand op = {};
        op.type = OpType_immediate;

        if (w == 1 && s == 1)
        {
            op.immediate = (int16_t) GetNextByte(cpu.IP);
        }
        else if (w == 1)
        {
            op.immediate = (int16_t) GetNextWord(cpu.IP);
        }
        else
        {
             op.immediate = (int16_t) GetNextByte(cpu.IP);
        }
        
        inst.operands[SRC] = op;
    }

    if (hasAddr)
    {
        EffectiveAddrExpression ex = {
            .calculationType = Effective_addr_direct_address,
            .displacement = (int16_t)GetNextWord(cpu.IP)
        };

        inst.operands[!d] = {
            .type = OpType_effectiveAddrCalc,
            .expression = ex
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
            // TODO: Need to replace magic '0' with proper constant like OpCode_Bits or something
            if (entry.bits[0].value == (currentByte >> (8 - entry.bits[0].count)))
            {
                Instruction result = Decode(cpu, entry);
                if (result.op)
                {
                    WriteToConsole(result);
                    break;
                }
            }

        }
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
