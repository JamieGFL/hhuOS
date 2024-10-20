#include "cpu6502.h"
#include "bus.h"
#include <stddef.h>

void cInit(struct cpu6502* cpu) {
    cpu->bus = NULL;
}

void cDestroy(struct cpu6502* cpu) {
    // No dynamic memory used yet
}

uint8_t cRead(struct cpu6502* cpu, uint16_t addr) {
    return cpu->bus->bRead(cpu->bus, addr, 0);
}

void cWrite(struct cpu6502* cpu, uint16_t addr, uint8_t val) {
    cpu->bus->bWrite(cpu->bus, addr, val);
}

void connectBus(struct cpu6502* cpu, struct bus* b) {
    cpu->bus = b;
}
