#pragma once
#include <cstdint>

// The size of the emulators memory in bytes
#define MEMORY_SIZE_BYTES 1000000

// The size of the prefetch buffer in bytes
#define PREFETCH_BUFFER_SIZE_BYTES 6

/** 
 * @brief Array that represents the physical main memory for an Intel 8086 machine 
 */
uint8_t memory[MEMORY_SIZE_BYTES];

/**
 * @brief Buffer that holds up to 6 bytes of prefetched instructions 
 */
uint8_t prefetchBuffer[PREFETCH_BUFFER_SIZE_BYTES];

/**
 * @brief Program Counter keeps track of where in 
 */
uint16_t PC = 0;
