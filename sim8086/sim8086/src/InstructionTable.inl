
/** Instruction Encoding Definitions */
#ifndef INST
#define INST(mnemonic, ...) { Op_##mnemonic, __VA_ARGS__ },
#endif

#ifndef INST_ALT
#define INST_ALT INST
#endif

/** Encoding Helpers */
// Note (Joe): The shift in a Literal bit is irrelevant because we are specifying exactly what the bits are. Not shift needed
// Format: Type, value, shift, count
#define B(type, bits) { type, 0b##bits, 0, (sizeof(#bits) - 1) }
#define Const(type, bits) { type, bits, 0, 0 }
#define OpExtension_bits(mnemonic) { OpExtension, Op_##mnemonic, 0, 0}
#define Imm_bits { Imm_bit, 0, 0, 0 }
#define Addr_bits { Addr_bit, 0, 0, 0 }
#define D_bits { D_bit, 0b1, 1, 1 }
#define Mod_bits {Mod_bit, 0b11, 6, 2 }
#define Reg_bits { Reg_bit, 0b111, 3, 3 }
#define Rm_bits { Rm_bit, 0b111, 0, 3 }
#define S_bits {S_bit, 0b1, 1, 1}

/* Instruction Table */
INST(MOV, { B(Op, 100010), D_bits, { W_bit, 1, 0, 1 }, Mod_bits, Rm_bits, Reg_bits } )
INST_ALT(MOV, { B(Op, 1100011), Const(D_bit, 0b0), { W_bit, 0b1, 0, 1 }, Mod_bits, Const(Const_bit, 0b000), Rm_bits, Imm_bits } )
INST_ALT(MOV, { B(Op, 1011), Const(D_bit, 0b1), { W_bit, 1, 3, 1 }, { Reg_bit, 0b111, 0, 3 }, Imm_bits } )
INST_ALT(MOV, { B(Op, 1010000), Const(D_bit, 0b1), {W_bit, 1, 0, 1}, Const(Reg_bit, 0b00), Addr_bits } )
INST_ALT(MOV, { B(Op, 1010001), Const(D_bit, 0b0), {W_bit, 1, 0, 1}, Const(Reg_bit, 0b00), Addr_bits } )

INST(ADD, {B(Op, 000000), D_bits, { W_bit, 0b1, 0, 1}, Mod_bits, Reg_bits, Rm_bits})
INST_ALT(ADD, { B(Op, 100000), S_bits, Const(D_bit, 0b0), { W_bit, 0b1, 0, 1 }, Mod_bits, OpExtension_bits(ADD), Rm_bits, Imm_bits })

#undef INST
#undef INST_ALT

#undef B
#undef D_bits
#undef Mod_bits
#undef Reg_bits
#undef Rm_bits
#undef Imm_bits