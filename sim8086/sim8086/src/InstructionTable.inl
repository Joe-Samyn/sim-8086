
/** Instruction Encoding Definitions */
#ifndef INST
#define INST(mnemonic, ...) { #mnemonic, __VA_ARGS__}
#endif

#ifndef INST_ALT
#define INST_ALT INST
#endif

/** Encoding Helpers */
#define B(bits) { Literal, 0b##bits, (8 - (sizeof(#bits) - 1)), (sizeof(#bits) - 1) }
#define D_bits { D_bit, 0b1, 1, 1 }
#define Mod_bits {Mod_bit, 0b11, 6, 2}
#define Reg_bits { Reg_bit, 0b111, 3, 3 }
#define Rm_bits { Rm_bit, 0b111, 0, 3 }

/* Instruction Table */
INST(MOV, { B(100010), D_bits, { W_bit, 0b1, 0, 1}, Mod_bits, Rm_bits, Reg_bits } ), \

#undef INST
#undef INST_ALT

#undef B
#undef D_bits