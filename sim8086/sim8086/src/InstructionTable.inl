
/** Instruction Encoding Definitions */
#ifndef INST
#define INST(mnemonic, ...) { #mnemonic, __VA_ARGS__}
#endif

#ifndef INST_ALT
#define INST_ALT INST
#endif

/** Encoding Helpers */
#define B(bits) { Literal, 0b##bits, (8 - (sizeof(#bits) - 1)), (sizeof(#bits) - 1) }
#define D_bits { D_bit, 0b10, 1, 1 }

/* Instruction Table */
INST(MOV, { B(100010), D_bits, { W_bit, 0b1, 0, 1} } ), \

#undef INST
#undef INST_ALT

#undef B
#undef D_bits