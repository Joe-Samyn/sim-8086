
/** Instruction Encoding Definitions */
#ifndef INST
#define INST(mnemonic, ...) { Op_##mnemonic, __VA_ARGS__ },
#endif

#ifndef INST_ALT
#define INST_ALT INST
#endif

#define NONE 0

/** Encoding Helpers */
// Note (Joe): The shift in a Literal bit is irrelevant because we are specifying exactly what the bits are. Not shift needed
// Format: Type, value, mask, shift, count
#define B(type, bits) { type, 0b##bits, NONE, NONE, (sizeof(#bits) - 1) }
#define Const(type, bits) { type, bits, NONE, NONE, NONE }
#define OpExtension(bits) { OpExtension, 0b##bits, 0b111, 3, 3 }
#define Inc(type) {type, NONE, NONE, NONE, NONE }
#define Imm { Imm_bit, NONE, NONE, NONE, NONE }
#define Addr { Addr_bit, NONE, NONE, NONE, NONE }
#define D { D_bit, NONE, 0b1, 1, 1 }
#define W { W_bit, NONE, 0b1, NONE, 1 }
#define Mod {Mod_bit, NONE, 0b11, 6, 2 }
#define Reg { Reg_bit, NONE, 0b111, 3, 3 }
#define Rm { Rm_bit, NONE, 0b111, 0, 3 }
#define S {S_bit, NONE, 0b1, 1, 1}
#define Accumulator {Acc_bit, 0b00, NONE, NONE, NONE }

/* Instruction Table */
INST(MOV, { B(Op, 100010), D, W, Mod, Rm, Reg } )
INST_ALT(MOV, { B(Op, 1100011), W, Mod, Const(Const_bit, 0b000), Rm, Imm } )
INST_ALT(MOV, { B(Op, 1011), { W_bit, NONE, 1, 3, 1 }, { Reg_bit, NONE, 0b111, 0, 3 }, Imm } )
INST_ALT(MOV, { B(Op, 1010000), Const(D_bit, 0b1), {W_bit, NONE, 1, 0, 1}, Const(Reg_bit, 0b00), Addr } )
INST_ALT(MOV, { B(Op, 1010001), Const(D_bit, 0b1), {W_bit, NONE, 1, 0, 1}, Const(Reg_bit, 0b00), Addr } )

INST(XCHG, { B(Op, 1000011), Const(D_bit, 0b1), W, Mod, Reg, Rm })
INST_ALT(XCHG, { B(Op, 10010), Const(D_bit, 0b0), Const(W_bit, 0b1), {Reg_bit, NONE, 0b111, 0, 3}, Accumulator })

INST(ADD, {B(Op, 000000), D, W, Mod, Reg, Rm})
INST_ALT(ADD, { B(Op, 100000), S, Const(D_bit, 0b0), W, Mod, OpExtension(000), Rm, Imm })
INST_ALT(ADD, { B(Op, 0001010), Const(D_bit, 0b0), W, Const(Reg_bit, 0b000), Imm })

INST(ADC, {B(Op, 000100), D, W, Mod, Reg, Rm})
INST_ALT(ADC, { B(Op, 100000), S, Const(D_bit, 0b0), W, Mod, OpExtension(010), Rm, Imm })
INST_ALT(ADC, { B(Op, 0001010), Const(D_bit, 0b0), W, Const(Reg_bit, 0b000), Imm })

INST(SUB, {B(Op, 001010), D, W, Mod, Reg, Rm})
INST_ALT(SUB, { B(Op, 100000), S, Const(D_bit, 0b0), W, Mod, OpExtension(101), Rm, Imm })
INST_ALT(SUB, { B(Op, 0010110), Const(D_bit, 0b0), W, Const(Reg_bit, 0b000), Imm })

INST(SBB, {B(Op, 000110), D, W, Mod, Reg, Rm})
INST_ALT(SBB, { B(Op, 100000), S, Const(D_bit, 0b0), W, Mod, OpExtension(011), Rm, Imm })
INST_ALT(SBB, { B(Op, 0001110), Const(D_bit, 0b0), W, Const(Reg_bit, 0b000), Imm })

INST(CMP, {B(Op, 001110), D, W, Mod, Reg, Rm})
INST_ALT(CMP, { B(Op, 100000), S, Const(D_bit, 0b0), W, Mod, OpExtension(111), Rm, Imm })
INST_ALT(CMP, { B(Op, 0011110), Const(D_bit, 0b0), W, Const(Reg_bit, 0b000), Imm })

INST(DEC, {B(Op, 1111111), Const(D_bit, 0b0), { W_bit, NONE, 0b1, 0, 1}, Mod, OpExtension(001), Rm})
INST_ALT(DEC, { B(Op, 01001), Const(D_bit, 0b1), Const(W_bit, 0b1), {Reg_bit, NONE, 0b111, 0, 3} })

INST(NEG, { B(Op, 1111011), Const(D_bit, 0b0), W, Mod, OpExtension(011), Rm })

INST(INC, { B(Op, 1111111), Const(D_bit, 0b0), W, Mod, OpExtension(000), Rm })
INST_ALT(INC, { B(Op, 01000), Const(D_bit, 0b1), Const(W_bit, 0b1), {Reg_bit, NONE, 0b111, 0, 3} })

INST(PUSH, { B(Op, 11111111), Const(D_bit, 0b0), Const(W_bit, 0b1), Mod, OpExtension(110), Rm })
INST_ALT(PUSH, { B(Op, 01010), Const(D_bit, 0b1), Const(W_bit, 0b1), {Reg_bit, NONE, 0b111, 0, 3} })

INST(POP, { B(Op, 10001111), Const(D_bit, 0b0), Const(W_bit, 0b1), Mod, OpExtension(000), Rm })
INST_ALT(POP, { B(Op, 01011), Const(D_bit, 0b1), Const(W_bit, 0b1), {Reg_bit, NONE, 0b111, 0, 3} })

INST(JMP, {B(Op, 11101001), Const(W_bit, 1), Inc(IPInc_bit) })
INST_ALT(JMP, { B(Op, 11101011), Const(W_bit, 0), Inc(IPInc_bit) })
INST_ALT(JMP, { B(Op, 11111111), Const(W_bit, 1), Mod, OpExtension(100), Rm })
INST_ALT(JMP, { B(Op, 11101011), Const(W_bit, 0), Inc(IPInc_bit), Inc(CSInc_bit) })
INST_ALT(JMP, { B(Op, 11111111), Const(W_bit, 1), Mod, OpExtension(101), Rm })

INST(JZ, { B(Op, 01110100), Const(W_bit, 0), Inc(IPInc_bit) })

INST(JNGE, { B(Op, 01111100), Const(W_bit, 0), Inc(IPInc_bit) })

INST(JNG, { B(Op, 01111110), Const(W_bit, 0), Inc(IPInc_bit) })

INST(JNAE, { B(Op, 01110010), Const(W_bit, 0), Inc(IPInc_bit) })

INST(JNA, { B(Op, 01110110), Const(W_bit, 0), Inc(IPInc_bit) })

INST(JPE, { B(Op, 01111010), Const(W_bit, 0), Inc(IPInc_bit) })

INST(JO, { B(Op, 01110000), Const(W_bit, 0), Inc(IPInc_bit) })

INST(JS, { B(Op, 01111000), Const(W_bit, 0), Inc(IPInc_bit) })

INST(JNZ, { B(Op, 01110101), Const(W_bit, 0), Inc(IPInc_bit) })

INST(JGE, { B(Op, 01111101), Const(W_bit, 0), Inc(IPInc_bit) })

INST(JG, { B(Op, 01111111), Const(W_bit, 0), Inc(IPInc_bit) })

INST(JAE, { B(Op, 01110011), Const(W_bit, 0), Inc(IPInc_bit) })

INST(JA, { B(Op, 01110111), Const(W_bit, 0), Inc(IPInc_bit) })

INST(JPO, { B(Op, 01111011), Const(W_bit, 0), Inc(IPInc_bit) })

INST(JNO, { B(Op, 01110001), Const(W_bit, 0), Inc(IPInc_bit) })

INST(JNS, { B(Op, 01111001), Const(W_bit, 0), Inc(IPInc_bit) })

INST(LOOP, { B(Op, 11100010), Const(W_bit, 0), Inc(IPInc_bit) })

INST(LOOPZ, { B(Op, 11100001), Const(W_bit, 0), Inc(IPInc_bit) })

INST(LOOPNZ, { B(Op, 11100000), Const(W_bit, 0), Inc(IPInc_bit) })

INST(JCXZ, { B(Op, 11100011), Const(W_bit, 0), Inc(IPInc_bit) })

INST(RET, { B(Op, 11000011), Const(W_bit, 0) })
INST_ALT(RET, { B(Op, 11000011), Const(W_bit, 1), Imm })

#undef INST
#undef INST_ALT

#undef B
#undef D
#undef Mod
#undef Reg
#undef Rm
#undef Imm