#include "Decode.h"
#include "Sim8086.h"

#include <assert.h>

/**
 * Determine the type of effective adddress expression encoded in MOD/RM bits and assign the correct type
 * to the operand.
 */
void DecodeEAExpression(uint8_t mod, uint8_t rm, Operand &op, SegmentedAddress &at) {
    // MOD should never == Register_mode if we are in this function
    assert(mod != Register_mode);

    if (rm == 0b110 && mod == Memory_mode_no_disp) {
        op.ea = Direct_address;
        op.displacement = (int16_t)ReadWordFromMemory(at);
        at.offset += 2;
    }
    else {
        op.ea = (EAType)(rm + 1);
        if (mod != Memory_mode_no_disp)
        {
            op.displacement = mod == Memory_mode_8_bit_disp ? (int16_t)((int8_t)ReadByteFromMemory(at)) : (int16_t)ReadWordFromMemory(at);
            at.offset += mod;
        }
    }
}


/**
 * TODO: Can be made much simpler. We already have register "codes" from the manual. Use the register code as is in manual. This will eliminate
 * the switch statement.
 */
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
    // NOTE: New code we are testing
    if (mod == Register_mode) {
        operand.type = OpType_register;
        operand.reg = {};
        DecodeRegister(rm, w, operand.reg);
    }
    else
    {
        operand.type = OpType_effectiveAddrCalc;
        DecodeEAExpression(mod, rm, operand, at);
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

/**
 * TODO: Decode is really a coordinator function. Worth looking into splitting out parts of decode into separate functions
 * for readability, testability.
 *
 * Ex:
 * // Write integration style test for all of Decode(..)
 * Decode:
 *  Setup function variables
 *
 *  ParseBits(..)   // Testable on its own
 *
 *  BuildInstruction(..)    // Testable on its own
 *
 *  return Instruction;
 *
 */
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

    /**
     * TODO: This is very verbose.. If not for performance, I would like to analyze this and see if we can make it simpler from a readability
     * standpoint.
     *
     * The number of conditionals in this flow do make me think there could be a high number of branch mispredictions. So, eliminating as
     * many branches as possible or making branches more predictable may be worth it.
     */
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
            Operand op = {};
            InterpretModRm(Memory_mode_no_disp, 0b110, w, op, at);
            inst.operands[!d] = op;
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

            inst.size = ComputePhysicalAddress(at) - inst.address;
            inst.operands[DEST] = {
                .type = OpType_jmp,
                .displacement = (int16_t)displacement
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
