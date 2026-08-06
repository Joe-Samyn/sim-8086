
#include <cstdint>
#include <fstream>
#include <iostream>
#include <cstdio>


/**
 * NOTE
 *  - Pay attention to JMP instructions when we get to execution stage. There are intersegment and within segment jumps. These 
 *      could get interesting since the instruction format is identical only different OpExtension bits. 
 */
#define ArrayCount(array) sizeof(array)/sizeof(array[0])

#define LO_BITS 0
#define HI_BITS 1
#define FULL_BITS 2

#define TRUE 1
#define FALSE 0

#define SRC 0
#define DEST 1

#define MEMORY_SIZE 1024 * 1024
#define BUFFER_SIZE 1000
#define INST_LENGTH 30

#define HasField(mask, field) (mask & (1 << field))
#define ComputePhysicalAddress(at) ((at.segment * 16) + at.offset)
#define IncrementAddress(at) at.offset++

static uint8_t Memory[MEMORY_SIZE];

struct SegmentedAddress {
    uint16_t segment;
    uint16_t offset;
};

SegmentedAddress Create(uint16_t segment, uint16_t offset) {
    return { 
        .segment=segment, 
        .offset=(uint16_t)offset 
    };
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

enum SegmentRegisters {
    CS,
    SS,
    DS,
    ES,

    Segment_count
};

struct CPU {
    uint16_t IP;
    uint16_t registers[Register_count];
    uint16_t segmentRegisters[Segment_count];
};

struct Program {
    uint32_t size;
    uint32_t startAddr;
    uint32_t endAddr;
};

uint8_t ReadByteFromMemory(SegmentedAddress at) {
    uint32_t address = ComputePhysicalAddress(at);
    return Memory[address];
}

uint16_t ReadWordFromMemory(SegmentedAddress at) {
    uint32_t address = ComputePhysicalAddress(at);
    uint8_t lo = Memory[address];
    uint16_t hi = Memory[address+1];
    return ((hi << 8) | lo);
}

uint8_t FetchNextInstructionByte(CPU &cpu) {
    SegmentedAddress at = { .segment=cpu.segmentRegisters[CS], .offset=cpu.IP};
    uint32_t address = ComputePhysicalAddress(at);
    cpu.IP++;
    return Memory[address];
}


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
    S_bit,
    Data_bit,
    Displacement_bit,

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

void DecodeEffectiveAddrExpression(uint8_t mod, uint8_t rm, EffectiveAddrExpression &expression, SegmentedAddress &at) 
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
                    expression.displacement = (int16_t)ReadWordFromMemory(at);
                    IncrementAddress(at);
                    IncrementAddress(at);
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
    uint8_t mask;
    uint8_t shift;
    uint8_t count;
};

enum Flags {
    Wide = (1 << 0),
    IPInc = (1 << 1),
    CSInc = (1 << 2),
    RmIsWide = (1 << 3)
};

struct Entry {
    Operation mnemonic;
    Bits bits[Field::Field_count];
    uint16_t flags;
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
    OpType_jmp,

    OpType_count
};

struct Jump {
    uint16_t ipAddress;
    uint16_t csAddress;
};

struct Operand {
    OperandType type;
    union {
        RegisterAccess reg;
        EffectiveAddrExpression expression;
        int16_t immediate;
        uint32_t address;
        Jump jmp;
    };
};

struct Instruction {
    Operation op;
    uint32_t address;
    uint16_t size;
    uint16_t flags;
    Operand operands[2];
};

Instruction DecodedInstructions[BUFFER_SIZE];     // String instruction buffer. Holds all ASM instructions to be printed 
static uint16_t DecodedInstIndex = 0;

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
            case Effective_addr_count:
            { 
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
        case OpType_jmp:
        {
            printf("$%+d", op.address);
            
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

void WriteToConsole() 
{
    // Print start label 
    for (int i = 0; i < DecodedInstIndex; i++)
    {
        Instruction inst = DecodedInstructions[i];
        // Print mnemonic/operation 
        printf("\t%s ", Mnemonics[inst.op]);

        // If either operand type is immediate, we should print size 
        if ((inst.operands[SRC].type == OpType_immediate || inst.operands[SRC].type == OpType_none) && inst.operands[DEST].type == OpType_effectiveAddrCalc)
        {
            printf("%s ", (inst.flags & Flags::Wide) == 0 ? "byte" : "word");
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
}

void Execute(Program &program)
{
    printf("Executing instructions...\n");
}

#define DIRECT_ADDRESS 0b110
void InterpretModRm(uint8_t mod, uint8_t rm, uint8_t w,  Operand &operand, SegmentedAddress &at)
{
    switch(mod)
    {
        case Memory_mode_no_disp:
            {
                operand.type = OpType_effectiveAddrCalc;
                EffectiveAddrExpression exp = {};
                DecodeEffectiveAddrExpression(mod, rm, exp, at);
                exp.hasDisplacement = FALSE;
                operand.expression = exp;
            } break;
        case Memory_mode_8_bit_disp:
            {
                operand.type = OpType_effectiveAddrCalc;
                EffectiveAddrExpression exp = {};
                DecodeEffectiveAddrExpression(mod, rm, exp, at);
                int8_t disp  = (int8_t)ReadByteFromMemory(at);
                IncrementAddress(at);
                exp.displacement = (int16_t)disp;
                exp.hasDisplacement = TRUE;
                operand.expression = exp;
            } break;
        case Memory_mode_16_bit_disp:
            {
                operand.type = OpType_effectiveAddrCalc;
                EffectiveAddrExpression exp = {};
                DecodeEffectiveAddrExpression(mod, rm, exp, at);
                exp.displacement = (int16_t)ReadWordFromMemory(at);
                IncrementAddress(at);
                IncrementAddress(at);
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

/**
* NOTE: Extracted from Decode to make Decode slightly easier to read. Provides no other function than that. 
*/
uint8_t ParseDataFromByte(Bits current, uint8_t &usedBits, SegmentedAddress &cursor) {

    uint8_t result = 0;

    // Checking for constant bits 
    if (current.count == 0)
    {
        // Get literal constant 
        result = current.value;
    }
    else
    {
        uint8_t byte = ReadByteFromMemory(cursor);
        if (usedBits >= 8)
        {
            IncrementAddress(cursor);
            byte = ReadByteFromMemory(cursor);
            usedBits = 0;
        }
        
        result = (byte >> current.shift) & current.mask;
    }

    return result;
}


Instruction Decode(Entry entry, SegmentedAddress &at)
{
    Instruction inst = {};
    inst.address = ComputePhysicalAddress(at);

    uint8_t bitsIndex = 0;
    uint8_t usedBits = 0;

    uint8_t extractedData[Field_count] = {};   
    uint32_t hasBits = 0;

    uint8_t valid = true;
    
    while(!(entry.bits[bitsIndex].field == Op && entry.bits[bitsIndex].count == 0) && valid)
    {  
        Bits currentBits = entry.bits[bitsIndex];
        uint8_t result = ParseDataFromByte(currentBits, usedBits, at);

        // Just break out if the opcode extension does not match because we are decoding the wrong instruction entry.
        if (currentBits.field == OpExtension && (result != currentBits.value))
        {
            valid = false;
        }
        
        extractedData[currentBits.field] = result;
        hasBits |= (1 << currentBits.field);
        usedBits += currentBits.count;
        bitsIndex++;
    }

    if (valid)
    {
        // NOTE: `at` is still pointing to the last byte used in the extraction loop. This incrmenet is required to move it 
        // to the next needed byte for instruction creation. 
        IncrementAddress(at);
        uint8_t d = extractedData[D_bit];
        uint8_t w = extractedData[W_bit];
        uint8_t s = extractedData[S_bit];
        
        inst.op = entry.mnemonic;
        inst.flags |= w;

        if (HasField(hasBits, Mod_bit))
        {
            uint8_t mod = extractedData[Mod_bit];
            uint8_t rm = extractedData[Rm_bit];

            Operand op = {};
            uint8_t isWide = (entry.flags & RmIsWide) ? 1 : w;
            InterpretModRm(mod, rm, isWide, op, at);
            inst.operands[!d] = op;
        }

        if (HasField(hasBits, Reg_bit))
        {
            uint8_t reg = extractedData[Reg_bit];

            RegisterAccess a = {};
            DecodeRegister(reg, w, a);
            Operand op = {
                .type = OpType_register,
                .reg = a		
            };

            inst.operands[d] = op;
        }

        if (HasField(hasBits, Imm_bit))
        {
            Operand op = {};
            op.type = OpType_immediate;

            bool isByte = (w == 1 && s == 1) || (w == 0);
            if (isByte)
            {
                int8_t imm = (int8_t) ReadByteFromMemory(at);
                IncrementAddress(at);
                op.immediate = (int16_t) imm;
            }
            else
            {
                op.immediate = (int16_t) ReadWordFromMemory(at);
                IncrementAddress(at);
                IncrementAddress(at);
            }
            
            inst.operands[SRC] = op;
        }

        if (HasField(hasBits, Addr_bit))
        {
            EffectiveAddrExpression ex = {
                .calculationType = Effective_addr_direct_address,
                .displacement = (int16_t)ReadWordFromMemory(at)
            };

            IncrementAddress(at);
            IncrementAddress(at);

            inst.operands[!d] = {
                .type = OpType_effectiveAddrCalc,
                .expression = ex
            };
        }

        if (HasField(hasBits, Displacement_bit))
        {   
            int16_t displacement = 0;
            if (w == 1)
            {
                displacement = (int16_t)ReadWordFromMemory(at);
                IncrementAddress(at);
                IncrementAddress(at);
            }
            else
            {
                int8_t inc = (int8_t)ReadByteFromMemory(at);
                IncrementAddress(at);
                displacement = (int16_t)inc;
            }

            // TODO: (joe) This needs to be fixed... Its a very terrible way to calculate the size of an instruction

            uint16_t size = ComputePhysicalAddress(at) - inst.address;
            inst.operands[DEST] = {
                .type = OpType_jmp,
                .address = (uint32_t)displacement + size
            };

            inst.flags |= IPInc;
        }

        if (HasField(hasBits, Data_bit))
        {
            int8_t imm = (int8_t) ReadByteFromMemory(at);
            IncrementAddress(at);
            inst.operands[!d] = {
                .type = OpType_immediate,
                .immediate = (int16_t) imm
            };

        }
    }

    return inst;
}

void Disassemble(Program &program)
{	
    CPU cpu = { 0 };

    while (cpu.IP <= program.endAddr)
    {
        uint8_t currentByte = FetchNextInstructionByte(cpu);
        Entry entry = {};

        // Search Instruction table for matching instruction 
        for (int i = 0; i < ArrayCount(InstructionTable); i++)
        {
            entry = InstructionTable[i];

            if (entry.bits[0].value == (currentByte >> (8 - entry.bits[0].count)))
            {
                SegmentedAddress at = Create(cpu.segmentRegisters[CS], cpu.IP - 1);
                Instruction result = Decode(entry, at);
                if (result.op)
                {
                    DecodedInstructions[DecodedInstIndex] = result;
                    DecodedInstIndex++;
                    cpu.IP = at.offset;
                    break;
                }
            }

        }
    }

    WriteToConsole();
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
