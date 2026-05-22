#pragma once
#include <cstdint>

// The size of the emulators memory in bytes
#define MEMORY_SIZE_BYTES 1000000

// The size of the prefetch buffer in bytes
#define PREFETCH_BUFFER_SIZE_BYTES 6

// The halt instruction to signify the end of a program.
// NOTE: This may not be the correct way to signal end of a program, but it is what is being used
// for now. 
#define HALT 0xF4

/**
 * @brief Struct that represents the state of the CPU, including memory and registers
 */
struct CPU {
	uint8_t memory[MEMORY_SIZE_BYTES];
	uint16_t PC;
};
