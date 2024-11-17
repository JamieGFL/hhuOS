
#include "lib/cpu6502/cpu6502.h"
#include "cartridge.h"
#include <stdint.h>

typedef struct ppu2C02 {
    cartridge* cart;

    // memory
    uint8_t nametable[2][1024];
    uint8_t palette[32];

} ppu2C02;

#ifdef __cplusplus
extern "C" {
#endif

void ppuInit(ppu2C02* ppuIn);

// writes to and reads from main bus
uint8_t ppuCpuRead(cpu6502* cpu, ppu2C02* ppuIn, uint16_t addr, int readOnly);
void ppuCpuWrite(cpu6502* cpu, ppu2C02* ppuIn, uint16_t addr, uint8_t val);

// writes to and reads from ppu bus
uint8_t ppuRead(ppu2C02* ppuIn, uint16_t addr, int readOnly);
void ppuWrite(ppu2C02* ppuIn, uint16_t addr, uint8_t val);


void connectCartridge(ppu2C02* ppuIn, cartridge* cart);
void ppuClock(ppu2C02* ppuIn);


#ifdef __cplusplus
}
#endif

