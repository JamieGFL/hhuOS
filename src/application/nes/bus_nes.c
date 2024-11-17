#include "bus_nes.h"

static bus_nes* nesBus;

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
//    bus->busBase->ram[0] = val;

        if(addr <= 0x1FFF){
            b->ram[addr & 0x07FF] = val;
        }
        else if(addr >= 0x2000 && addr <= 0x3FFF){
            cpuWrite(nesBus->cpu, nesBus->ppu, addr & 0x0007, val);
        }
}

uint8_t nes_bus_Read(bus* b, uint16_t addr, int readOnly) {
    uint8_t data = 0x00;

    if(addr <= 0x1FFF){
        data = b->ram[addr & 0x07FF];
    }
    else if(addr >= 0x2000 && addr <= 0x3FFF){
        data = cpuRead(nesBus->cpu, nesBus->ppu, addr & 0x0007, readOnly);
    }

    if(readOnly) {
        readOnly = 0;
    }

    return data;
}

void setBus(bus_nes *b) {
    nesBus = b;
}