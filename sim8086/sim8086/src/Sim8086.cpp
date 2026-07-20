
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


static uint8_t Memory[MEMORY_SIZE];

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
    IPInc_bit,
    CSInc_bit,
    Acc_bit,
    Dx_bit,
    Data8_bit,

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
    uint8_t mask;
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
    OpType_label,

    OpType_count
};

struct Operand {
    OperandType type;
    union {
        RegisterAccess reg;
        EffectiveAddrExpression expression;
        int16_t immediate;
        uint16_t label;
    };
};

struct Instruction {
    Operation op;
    uint8_t w;
    uint8_t d;
    int16_t ipInc;
    int16_t csInc;
    Operand operands[2];
};

Instruction DecodedInstructions[BUFFER_SIZE];     // String instruction buffer. Holds all ASM instructions to be printed 
uint16_t DecodedInstIp[BUFFER_SIZE];            // The IP for each instruction to be printed. 
uint16_t LabelId[BUFFER_SIZE];          // Container holding labels and the correspinding IP. Used during printing to determine where labels should be printed in the output.
static uint8_t LabelCount = 1;
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
        case OpType_label:
        {
            printf("L%d", op.label);
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
        // Check if a label needs to be printed first. 
        uint16_t instIP = DecodedInstIp[i];
        if (LabelId[instIP])
        {
            printf("L%d: \n", LabelId[instIP]);
        }

        Instruction inst = DecodedInstructions[i];
        // Print mnemonic/operation 
        printf("\t%s ", Mnemonics[inst.op]);

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
}


/**
 * Used to determine if all bits in an instruction have been decoded. 
 * If all fields are 0, then its an uninitialized struct and is the end
 * of the bits array. 
 */
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
    uint8_t extractedBits[Field_count] = { 0 };     // Actual bits extracted from byte stream using entry Bits
    uint8_t expectedValues[Field_count] = { 0 };    // The 'value' property from all the entry Bits, used to verify constants (Values) if needed
    uint8_t hasFields[Field_count] = { 0 };         // Tracks which fields an entry actually has 
    
    // TODO: We need to make this loop be purely about extracting bits. The OpExtension check needs to move out somehow. 
    while(IsBitsDefined(entry.bits[bitsIndex]))
    {  

        Bits bit = entry.bits[bitsIndex];
        uint8_t result;

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
            
            result = (byte >> bit.shift) & bit.mask;
            
        }
        
        hasFields[bit.field] = TRUE;
        extractedBits[bit.field] = result;
        expectedValues[bit.field] = bit.value;
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
    uint8_t hasOpExt = hasFields[OpExtension];
    uint8_t hasIpIncr = hasFields[IPInc_bit];
    uint8_t hasCsInc = hasFields[CSInc_bit];
    uint8_t hasAccumulator = hasFields[Acc_bit];
    uint8_t hasData8 = hasFields[Data8_bit];
    uint8_t hasDx = hasFields[Dx_bit];

    uint8_t d = extractedBits[D_bit];
    uint8_t w = extractedBits[W_bit];
    uint8_t s = extractedBits[S_bit];
    uint8_t mod = extractedBits[Mod_bit];
    uint8_t rm = extractedBits[Rm_bit];
    uint8_t reg = extractedBits[Reg_bit];
    uint8_t opExt = extractedBits[OpExtension];

    // If the instruction has an Opcode Extension, we need to ensure the opcode extension also matches the instruction entry. 
    if (hasOpExt)
    {
        if (opExt != expectedValues[OpExtension])
        {
            return {};
        }
    }
    
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
            int8_t imm = (int8_t) GetNextByte(cpu.IP);
            op.immediate = (int16_t) imm;
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

    if (hasIpIncr)
    {   

        if (w == 1)
        {
            inst.ipInc = (int16_t)GetNextWord(cpu.IP);
        }
        else
        {
            int8_t inc = (int8_t)GetNextByte(cpu.IP);
            inst.ipInc = (int16_t)inc;
        }

        uint16_t labelIndex = cpu.IP + inst.ipInc;
        if (LabelId[labelIndex] == 0)
        {
            LabelId[labelIndex] = LabelCount++;
        }

        inst.operands[DEST] = {
            .type = OpType_label,
            .label = LabelId[labelIndex]
        };
    }

    if (hasCsInc)
    {
        inst.csInc = (int16_t)GetNextWord(cpu.IP);
    }

    if (hasAccumulator)
    {
        uint8_t offset = (w == 1) ? FULL_BITS : LO_BITS;
        inst.operands[!d] = {
            .type = OpType_register, 
            .reg = {
                .index = Register_a,
                .offset = offset
            }
        };
    }

    if (hasData8)
    {
        inst.operands[d] = {
            .type = OpType_immediate,
            .immediate = (int16_t) GetNextByte(cpu.IP)
        };
    }

    if (hasDx)
    {
        inst.operands[d] = {
            .type = OpType_register,
            .reg = {
                .index = Register_d,
                .offset = FULL_BITS
            }
        };
    }

    return inst;
}


/**
 * Instruction Table should drive decode, not vice versa
 * An entry tells you how many bits/bytes to pull from the stream
 */
/*
    TODO: We should determine which buffer size is most optimal. Once the buffer is full, just flush it and start over. 
        This would allow the program to be as long as we want without any limit on the number of instructions. Also, batching 
        is probably more performant than leaving a giant buffer of decoded instructions in memory. 
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
                CPU preDecodeState = cpu;
                Instruction result = Decode(cpu, entry);
                if (result.op)
                {
                    DecodedInstructions[DecodedInstIndex] = result;
                    DecodedInstIp[DecodedInstIndex] = preDecodeState.IP - 1;    // IP always points to the next byte. If we want the byte that the instruction started at, we need to subtract 1. 
                    DecodedInstIndex++;
                    break;
                }
                
                // Restore predecode CPU state before looking for another entry match
                cpu = preDecodeState;
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
