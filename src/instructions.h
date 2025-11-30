#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "types.h"

struct InstructionInfo {
    char* mnemonic;
    char* description;
    byte opcode;
};

// Defining the instruction table
#define INST_TABLE \
    X("MOV", "Register/memory to/from register", 0b100010) \
    X("MOV", "Immediate to register/memory", 0b1100011)
#endif

#define X(mnemonic, desc, opcode) \
    { mnemonic, desc, opcode },

    const struct InstructionInfo InstructionTable[] = {
        INST_TABLE
    };
    
#undef X