
#include "lib/cpu6502/cpu6502.h"
#include <stdint.h>

typedef struct ppu2C02 {
    int placeholder;
} ppu2C02;

#ifdef __cplusplus
extern "C" {
#endif

void ppuInit(ppu2C02* ppuIn);

// writes to and reads from main bus
uint8_t cpuRead(cpu6502* cpu, ppu2C02* ppuIn, uint16_t addr, int readOnly);
void cpuWrite(cpu6502* cpu, ppu2C02* ppuIn, uint16_t addr, uint8_t val);

// writes to and reads from ppu bus
uint8_t ppuRead(ppu2C02* ppuIn, uint16_t addr, int readOnly);
void ppuWrite(ppu2C02* ppuIn, uint16_t addr, uint8_t val);


#ifdef __cplusplus
}
#endif

