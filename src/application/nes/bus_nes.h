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

void bInit(bus_nes* b, bus* bus);


#endif //HHUOS_BUS_NES_H
