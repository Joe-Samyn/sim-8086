
/** Instruction Encoding Definitions */
#ifndef INST
#define INST(mnemonic, ...) { #mnemonic, __VA_ARGS__}
#endif

#ifndef INST_ALT
#define INST_ALT INST
#endif

/** Encoding Helpers */
// Note (Joe): The shift in a Literal bit is irrelevant because we are specifying exactly what the bits are. Not shift needed
// Format: Type, isLiteral, value, shift, count
#define B(type, bits) { type, true, 0b##bits, 0, (sizeof(#bits) - 1) }
#define D_bits { D_bit, false, 0b1, 1, 1 }
#define Mod_bits {Mod_bit, false, 0b11, 6, 2}
#define Reg_bits { Reg_bit, false, 0b111, 3, 3 }
#define Rm_bits { Rm_bit, false, 0b111, 0, 3 }

/* Instruction Table */
// NOTE (Joe): To prevent GetNextByte from progressing prematurely, all nonincrementing count bits (Imm, Addr, etc.) need to go towards front.
INST(MOV, { B(Op, 100010), D_bits, { W_bit, false, 0b1, 0, 1}, Mod_bits, Rm_bits, Reg_bits } ), \
INST_ALT(MOV, { B(Op, 1100011), { Imm_bit, false, 0, 0, 0 }, {D_bit, true, 0b0, 0, 0}, { W_bit, 0b1, 0, 1 }, Mod_bits, B(Const_bit, 000), Rm_bits } ) \

#undef INST
#undef INST_ALT

#undef B
#undef D_bits
#undef Mod_bits
#undef Reg_bits
#undef Rm_bits