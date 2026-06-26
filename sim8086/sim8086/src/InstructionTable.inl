
/** Instruction Encoding Definitions */
#ifndef INST
#define INST(mnemonic, ...) { #mnemonic, __VA_ARGS__}
#endif

#ifndef INST_ALT
#define INST_ALT INST
#endif

/** Encoding Helpers */
// Note (Joe): The shift in a Literal bit is irrelevant because we are specifying exactly what the bits are. Not shift needed
#define B(bits) { Literal, 0b##bits, 0, (sizeof(#bits) - 1) }
#define D_bits { D_bit, 0b1, 1, 1 }
#define Mod_bits {Mod_bit, 0b11, 6, 2}
#define Reg_bits { Reg_bit, 0b111, 3, 3 }
#define Rm_bits { Rm_bit, 0b111, 0, 3 }

/* Instruction Table */
INST(MOV, { B(100010), D_bits, { W_bit, 0b1, 0, 1}, Mod_bits, Rm_bits, Reg_bits } ), \
INST_ALT(MOV, { B(1100011), { W_bit, 0b1, 0, 1 }, Mod_bits, B(000), Rm_bits , { Imm_bit, 0, 0, 0 } } ) \

#undef INST
#undef INST_ALT

#undef B
#undef D_bits