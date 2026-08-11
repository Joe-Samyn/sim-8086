#include "Decode.h"


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