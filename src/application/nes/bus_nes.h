#ifndef HHUOS_BUS_NES_H
#define HHUOS_BUS_NES_H

#include <stdint.h>
#include "lib/cpu6502/cpu6502.h"
#include "ppu2C02.h"
#include "cartridge.h"


typedef struct {
    // how many cycles have passed
    uint32_t clockCount;

    // components
    cpu6502* cpu;
    ppu2C02* ppu;
    cartridge* cart;

    // bus
    bus* busBase;

} bus_nes;

#ifdef __cplusplus
extern "C" {
#endif

void bInit(bus_nes* b, bus* bus, cpu6502* cpu);
void setBus(bus_nes *b);

void nes_bus_Write(bus* b, uint16_t addr, uint8_t val);
uint8_t nes_bus_Read(bus* b, uint16_t addr, int readOnly);


void insertCartridge(bus_nes* b, cartridge* cart);

void busReset(bus_nes* b);

void busClock(bus_nes* b);
#ifdef __cplusplus
}
#endif


#endif //HHUOS_BUS_NES_H
