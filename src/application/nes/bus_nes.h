#ifndef HHUOS_BUS_NES_H
#define HHUOS_BUS_NES_H

#include <stdint.h>
#include "lib/cpu6502/cpu6502.h"


typedef struct {
    // components
    cpu6502* cpu;

    // bus
    bus* busBase;

} bus_nes;



void bInit(bus_nes* b, bus* bus, cpu6502* cpu);

void nes_bus_Write(bus* b, uint16_t addr, uint8_t val);
uint8_t nes_bus_Read(bus* b, uint16_t addr, int readOnly);


#endif //HHUOS_BUS_NES_H
