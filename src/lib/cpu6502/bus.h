#pragma once
#include <cstdint>
#include "cpu6502.h"

struct bus {
    // Components
    struct cpu6502 cpu;

    // Fake RAM (64KB)
    uint8_t ram[64 * 1024];

    uint8_t (*bRead)(struct bus* b, uint16_t addr, int readOnly);
    void (*bWrite)(struct bus* b, uint16_t addr, uint8_t val);
};

// Write to memory
void bWrite(struct bus* b, uint16_t addr, uint8_t val);

// Read from memory
uint8_t bRead(struct bus* b, uint16_t addr, int readOnly);

