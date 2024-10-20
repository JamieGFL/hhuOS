#pragma once
#include <stdint.h>

struct bus;

struct cpu6502 {
    struct bus* bus;

    uint8_t (*cRead)(struct cpu6502* cpu, uint16_t addr);
    void (*cWrite)(struct cpu6502* cpu, uint16_t addr, uint8_t val);
};

void cInit(struct cpu6502* cpu);

void cDestroy(struct cpu6502* cpu);

void connectBus(struct cpu6502* cpu, struct bus* b);

// write to memory through bus
uint8_t cRead(struct cpu6502_2* cpu, uint16_t addr);

// read from memory through bus
void cWrite(struct cpu6502_2* cpu, uint16_t addr, uint8_t val);
