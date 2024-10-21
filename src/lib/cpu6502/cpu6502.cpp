#include "cpu6502.h"
#include "bus.h"
#include <stddef.h>

void cInit(struct cpu6502* cpu) {
    cpu->bus = NULL;
    cpu->A = 0x00;
    cpu->X = 0x00;
    cpu->Y = 0x00;
    cpu->SP = 0x00;
    cpu->PC = 0x0000;
    cpu->status = 0x00;

    cpu->fetched = 0x00;

    cpu->addr_abs = 0x0000;
    cpu->addr_rel = 0x00;
    cpu->opcode = 0x00;
    cpu->cycles = 0;

}

void cDestroy(cpu6502* cpu) {
    // No dynamic memory used yet
}

uint8_t cRead(cpu6502* cpu, uint16_t addr) {
    return cpu->bus->bRead(cpu->bus, addr, 0);
}

void cWrite(cpu6502* cpu, uint16_t addr, uint8_t val) {
    cpu->bus->bWrite(cpu->bus, addr, val);
}

void connectBus(cpu6502* cpu,bus* b) {
    cpu->bus = b;
}
