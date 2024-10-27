#ifndef HHUOS_BUS_NES_H
#define HHUOS_BUS_NES_H

#include <stdint.h>
#include <bus_interface.h>
#include <cpu6502.h>

typedef struct {
    // bus
    bus bus;

    // Components
    cpu6502 cpu;

} bus_nes;

void bInit(bus_nes* b);


#endif //HHUOS_BUS_NES_H
