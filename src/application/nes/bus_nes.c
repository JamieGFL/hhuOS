#include "bus_nes.h"

void nes_bus_Write(bus* b, uint16_t addr, uint8_t val);
uint8_t nes_bus_Read(bus* b, uint16_t addr, int readOnly);

void bInit(bus_nes* b, bus* bus, cpu6502* cpu) {
    b->busBase = bus;

    // set function pointers
    b->busBase->bWrite = nes_bus_Write;
    b->busBase->bRead = nes_bus_Read;

    for (unsigned int i = 0; i < sizeof(b->busBase->ram); i++) {
        b->busBase->ram[i] = 0x00;
    }

    b->cpu = cpu;

    connectBus(b->cpu, b->busBase);
}

void nes_bus_Write(bus* b, uint16_t addr, uint8_t val) {
        b->ram[addr] = val;
}

uint8_t nes_bus_Read(bus* b, uint16_t addr, int readOnly) {
    return b->ram[addr];
    if(readOnly) {
        readOnly = 0;
    }

    return 0x00;
}