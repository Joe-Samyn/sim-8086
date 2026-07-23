
/** Instruction Encoding Definitions */
#ifndef INST
#define INST(mnemonic, ...) { Op_##mnemonic, __VA_ARGS__ },
#endif

#ifndef INST_ALT
#define INST_ALT INST
#endif

#define NONE 0
#define ACCUMULATOR 0b00

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
#define Data8 {Data8_bit, NONE, NONE, NONE, NONE }
#define Dx {Dx_bit, NONE, NONE, NONE, NONE }
#define ImpW(value) {W_bit, value, NONE, NONE, NONE }
#define ImpReg(register) { Reg_bit, register, NONE, NONE, NONE }
#define ImpD(value) {D_bit, value, NONE, NONE, NONE }

/* Instruction Table */
INST(MOV, { B(Op, 100010), D, W, Mod, Rm, Reg } )
INST_ALT(MOV, { B(Op, 1100011), W, Mod, OpExtension(000), Rm, Imm } )
INST_ALT(MOV, { B(Op, 1011), ImpD(0b1), { W_bit, NONE, 1, 3, 1 }, { Reg_bit, NONE, 0b111, 0, 3 }, Imm } )
INST_ALT(MOV, { B(Op, 1010000), ImpD(0b1), {W_bit, NONE, 1, 0, 1}, ImpReg(ACCUMULATOR), Addr } )
INST_ALT(MOV, { B(Op, 1010001), ImpD(0b1), {W_bit, NONE, 1, 0, 1}, ImpReg(ACCUMULATOR), Addr } )

INST(XCHG, { B(Op, 1000011), ImpD(0b1), W, Mod, Reg, Rm })
INST_ALT(XCHG, { B(Op, 10010), ImpD(0b0), ImpW(0b1), {Reg_bit, NONE, 0b111, 0, 3}, Accumulator })

INST(IN, {B(Op, 1110010), ImpD(0b0), W, Accumulator, Data8 })
INST_ALT(IN, { B(Op, 1110110), ImpD(0b0), W, Accumulator, Dx})

INST(OUT, { B(Op, 1110011), ImpD(0b1), W, Accumulator, Data8 })
INST_ALT(OUT, { B(Op, 1110111), ImpD(0b1), W, Accumulator, Dx})

INST(ADD, {B(Op, 000000), D, W, Mod, Reg, Rm})
INST_ALT(ADD, { B(Op, 100000), S, ImpD(0b0), W, Mod, OpExtension(000), Rm, Imm })
INST_ALT(ADD, { B(Op, 0001010), ImpD(0b0), W, ImpReg(ACCUMULATOR), Imm })

INST(ADC, {B(Op, 000100), D, W, Mod, Reg, Rm})
INST_ALT(ADC, { B(Op, 100000), S, ImpD(0b0), W, Mod, OpExtension(010), Rm, Imm })
INST_ALT(ADC, { B(Op, 0001010), ImpD(0b0), W, ImpReg(ACCUMULATOR), Imm })

INST(SUB, {B(Op, 001010), D, W, Mod, Reg, Rm})
INST_ALT(SUB, { B(Op, 100000), S, ImpD(0b0), W, Mod, OpExtension(101), Rm, Imm })
INST_ALT(SUB, { B(Op, 0010110), ImpD(0b0), W, ImpReg(ACCUMULATOR), Imm })

INST(SBB, {B(Op, 000110), D, W, Mod, Reg, Rm})
INST_ALT(SBB, { B(Op, 100000), S, ImpD(0b0), W, Mod, OpExtension(011), Rm, Imm })
INST_ALT(SBB, { B(Op, 0001110), ImpD(0b0), W, ImpReg(ACCUMULATOR), Imm })

INST(CMP, {B(Op, 001110), D, W, Mod, Reg, Rm})
INST_ALT(CMP, { B(Op, 100000), S, ImpD(0b0), W, Mod, OpExtension(111), Rm, Imm })
INST_ALT(CMP, { B(Op, 0011110), ImpD(0b0), W, ImpReg(ACCUMULATOR), Imm })

INST(DEC, {B(Op, 1111111), ImpD(0b0), { W_bit, NONE, 0b1, 0, 1}, Mod, OpExtension(001), Rm})
INST_ALT(DEC, { B(Op, 01001), ImpD(0b1), ImpW(0b1), {Reg_bit, NONE, 0b111, 0, 3} })

INST(NEG, { B(Op, 1111011), ImpD(0b0), W, Mod, OpExtension(011), Rm })

INST(INC, { B(Op, 1111111), ImpD(0b0), W, Mod, OpExtension(000), Rm })
INST_ALT(INC, { B(Op, 01000), ImpD(0b1), ImpW(0b1), {Reg_bit, NONE, 0b111, 0, 3} })

INST(PUSH, { B(Op, 11111111), ImpD(0b0), ImpW(0b1), Mod, OpExtension(110), Rm })
INST_ALT(PUSH, { B(Op, 01010), ImpD(0b1), ImpW(0b1), {Reg_bit, NONE, 0b111, 0, 3} })

INST(POP, { B(Op, 10001111), ImpD(0b0), ImpW(0b1), Mod, OpExtension(000), Rm })
INST_ALT(POP, { B(Op, 01011), ImpD(0b1), ImpW(0b1), {Reg_bit, NONE, 0b111, 0, 3} })

INST(JMP, {B(Op, 11101001), ImpW(1), Inc(IPInc_bit) })
INST_ALT(JMP, { B(Op, 11101011), ImpW(0), Inc(IPInc_bit) })
INST_ALT(JMP, { B(Op, 11111111), ImpW(1), Mod, OpExtension(100), Rm })
INST_ALT(JMP, { B(Op, 11101011), ImpW(0), Inc(IPInc_bit), Inc(CSInc_bit) })
INST_ALT(JMP, { B(Op, 11111111), ImpW(1), Mod, OpExtension(101), Rm })

INST(JZ, { B(Op, 01110100), ImpW(0), Inc(IPInc_bit) })

INST(JNGE, { B(Op, 01111100), ImpW(0), Inc(IPInc_bit) })

INST(JNG, { B(Op, 01111110), ImpW(0), Inc(IPInc_bit) })

INST(JNAE, { B(Op, 01110010), ImpW(0), Inc(IPInc_bit) })

INST(JNA, { B(Op, 01110110), ImpW(0), Inc(IPInc_bit) })

INST(JPE, { B(Op, 01111010), ImpW(0), Inc(IPInc_bit) })

INST(JO, { B(Op, 01110000), ImpW(0), Inc(IPInc_bit) })

INST(JS, { B(Op, 01111000), ImpW(0), Inc(IPInc_bit) })

INST(JNZ, { B(Op, 01110101), ImpW(0), Inc(IPInc_bit) })

INST(JGE, { B(Op, 01111101), ImpW(0), Inc(IPInc_bit) })

INST(JG, { B(Op, 01111111), ImpW(0), Inc(IPInc_bit) })

INST(JAE, { B(Op, 01110011), ImpW(0), Inc(IPInc_bit) })

INST(JA, { B(Op, 01110111), ImpW(0), Inc(IPInc_bit) })

INST(JPO, { B(Op, 01111011), ImpW(0), Inc(IPInc_bit) })

INST(JNO, { B(Op, 01110001), ImpW(0), Inc(IPInc_bit) })

INST(JNS, { B(Op, 01111001), ImpW(0), Inc(IPInc_bit) })

INST(LOOP, { B(Op, 11100010), ImpW(0), Inc(IPInc_bit) })

INST(LOOPZ, { B(Op, 11100001), ImpW(0), Inc(IPInc_bit) })

INST(LOOPNZ, { B(Op, 11100000), ImpW(0), Inc(IPInc_bit) })

INST(JCXZ, { B(Op, 11100011), ImpW(0), Inc(IPInc_bit) })

INST(RET, { B(Op, 11000011), ImpW(0) })
INST_ALT(RET, { B(Op, 11000011), ImpW(1), Imm })

#undef INST
#undef INST_ALT

#undef B
#undef D
#undef Mod
#undef Reg
#undef Rm
#undef Imm