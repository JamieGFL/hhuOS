#pragma once


#include "cpu6502.h"

typedef struct bus {
    // Components
    cpu6502 cpu;

    // Write to memory
    void (*bWrite)(struct bus* b, uint16_t addr, uint8_t val);
    // Read from memory
    uint8_t (*bRead)(struct bus* b, uint16_t addr, int readOnly);

    // Fake RAM (64KB)
    uint8_t ram[64 * 1024];
} bus;

void bWrite(bus* b, uint16_t addr, uint8_t val);
uint8_t bRead(bus* b, uint16_t addr, int readOnly);
