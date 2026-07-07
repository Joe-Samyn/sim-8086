
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
#define OpExtension_bits(bits) { OpExtension, 0b##bits, 3, 3 }
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
INST_ALT(ADD, { B(Op, 100000), S_bits, Const(D_bit, 0b0), { W_bit, 0b1, 0, 1 }, Mod_bits, OpExtension_bits(000), Rm_bits, Imm_bits })
INST_ALT(ADD, { B(Op, 0001010), Const(D_bit, 0b0), { W_bit, 0b1, 0, 1 }, Const(Reg_bit, 0b000), Imm_bits })

INST(ADC, {B(Op, 000100), D_bits, { W_bit, 0b1, 0, 1}, Mod_bits, Reg_bits, Rm_bits})
INST_ALT(ADC, { B(Op, 100000), S_bits, Const(D_bit, 0b0), { W_bit, 0b1, 0, 1 }, Mod_bits, OpExtension_bits(010), Rm_bits, Imm_bits })
INST_ALT(ADC, { B(Op, 0001010), Const(D_bit, 0b0), { W_bit, 0b1, 0, 1 }, Const(Reg_bit, 0b000), Imm_bits })

INST(SUB, {B(Op, 001010), D_bits, { W_bit, 0b1, 0, 1}, Mod_bits, Reg_bits, Rm_bits})
INST_ALT(SUB, { B(Op, 100000), S_bits, Const(D_bit, 0b0), { W_bit, 0b1, 0, 1 }, Mod_bits, OpExtension_bits(101), Rm_bits, Imm_bits })
INST_ALT(SUB, { B(Op, 0010110), Const(D_bit, 0b0), { W_bit, 0b1, 0, 1 }, Const(Reg_bit, 0b000), Imm_bits })

INST(SBB, {B(Op, 000110), D_bits, { W_bit, 0b1, 0, 1}, Mod_bits, Reg_bits, Rm_bits})
INST_ALT(SBB, { B(Op, 100000), S_bits, Const(D_bit, 0b0), { W_bit, 0b1, 0, 1 }, Mod_bits, OpExtension_bits(011), Rm_bits, Imm_bits })
INST_ALT(SBB, { B(Op, 0001110), Const(D_bit, 0b0), { W_bit, 0b1, 0, 1 }, Const(Reg_bit, 0b000), Imm_bits })

INST(CMP, {B(Op, 001110), D_bits, { W_bit, 0b1, 0, 1}, Mod_bits, Reg_bits, Rm_bits})
INST_ALT(CMP, { B(Op, 100000), S_bits, Const(D_bit, 0b0), { W_bit, 0b1, 0, 1 }, Mod_bits, OpExtension_bits(111), Rm_bits, Imm_bits })
INST_ALT(CMP, { B(Op, 0011110), Const(D_bit, 0b0), { W_bit, 0b1, 0, 1 }, Const(Reg_bit, 0b000), Imm_bits })

INST(DEC, {B(Op, 1111111), Const(D_bit, 0b0), { W_bit, 0b1, 0, 1}, Mod_bits, OpExtension_bits(001), Rm_bits})
INST_ALT(DEC, { B(Op, 01001), Const(D_bit, 0b1), Const(W_bit, 0b1), {Reg_bit, 0b111, 0, 3} })

INST(NEG, { B(Op, 1111011), Const(D_bit, 0b0), { W_bit, 0b1, 0, 1 }, Mod_bits, OpExtension_bits(011), Rm_bits })

INST(INC, { B(Op, 1111111), Const(D_bit, 0b0), { W_bit, 0b1, 0, 1 }, Mod_bits, OpExtension_bits(000), Rm_bits })
INST_ALT(INC, { B(Op, 01000), Const(D_bit, 0b1), Const(W_bit, 0b1), {Reg_bit, 0b111, 0, 3} })

INST(PUSH, { B(Op, 11111111), Const(D_bit, 0b0), Const(W_bit, 0b1), Mod_bits, OpExtension_bits(110), Rm_bits })
INST_ALT(PUSH, { B(Op, 01010), Const(D_bit, 0b1), Const(W_bit, 0b1), {Reg_bit, 0b111, 0, 3} })

INST(POP, { B(Op, 10001111), Const(D_bit, 0b0), Const(W_bit, 0b1), Mod_bits, OpExtension_bits(000), Rm_bits })
INST_ALT(POP, { B(Op, 01011), Const(D_bit, 0b1), Const(W_bit, 0b1), {Reg_bit, 0b111, 0, 3} })

#undef INST
#undef INST_ALT

#undef B
#undef D_bits
#undef Mod_bits
#undef Reg_bits
#undef Rm_bits
#undef Imm_bits