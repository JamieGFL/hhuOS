#include "bus_nes.h"
#include <stdlib.h>
#include <stdint.h>

static void nes_bus_Write(bus* b, uint16_t addr, uint8_t val);
static uint8_t nes_bus_Read(bus* b, uint16_t addr, int readOnly);

void bInit(bus_nes* b) {
    b->bus

    // set function pointers
    b->bus.bWrite = nes_bus_Write;
    b->bus.bRead = nes_bus_Read;

    for (unsigned int i = 0; i < sizeof(b->ram); i++) {
        b->bus.ram[i] = 0x00;
    }
}

static void nes_bus_Write(bus* b, uint16_t addr, uint8_t val) {
    if (addr >= 0x0000 && addr <= 0xFFFF) {
        b->ram[addr] = val;
    }
}

static uint8_t nes_bus_Read(bus* b, uint16_t addr, int readOnly) {
    if (addr >= 0x0000 && addr <= 0xFFFF) {
        return b->ram[addr];
    }

    return 0x00;
}