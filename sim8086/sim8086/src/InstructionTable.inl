#pragma once 



/** Instruction Encoding Definitions */
#ifndef INST
#define INST(mnemonic, ...) { #mnemonic, __VA_ARGS__}
#endif

#ifndef INSTAL
#define INST_ALT INST
#endif

/** Encoding Helpers */
#define B(bits) { (Literal), (0b##bits), (8 - (sizeof(#bits) - 1)), (sizeof(#bits) - 1) }
#define D_bits { D_bit, 0b1, 1, 1 }

/* Instruction Table */
#define Instructions \
	INST(MOV, { B(100010), D_bits, { W_bit, 0b10, 2, 1} } ), \

#undef INST
#undef INST_ALT