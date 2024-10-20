#include "bus.h"


void bInit(struct bus* b) {
    for (unsigned int i = 0; i < sizeof(b->ram); i++) {
        b->ram[i] = 0x00;
    }
}

void bWrite(struct bus* b, uint16_t addr, uint8_t val) {
    if (addr >= 0x0000 && addr <= 0xFFFF) {
        b->ram[addr] = val;
    }
}

uint8_t bRead(struct bus* b, uint16_t addr, int readOnly) {
    if (addr >= 0x0000 && addr <= 0xFFFF) {
        return b->ram[addr];
    }

    return 0x00;
}