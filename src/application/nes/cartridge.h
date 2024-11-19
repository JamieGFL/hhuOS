#ifndef HHUOS_CARTRIDGE_H
#define HHUOS_CARTRIDGE_H
#include "lib/cpu6502/cpu6502.h"
#include "mapper.h"

typedef struct ppu2C02 ppu2C02;
typedef struct mapper mapper;
typedef struct {
    uint8_t *data;
    uint32_t size;
} memoryChunk;

typedef struct cartridge {
    // memory
    memoryChunk prg;
    memoryChunk chr;

    uint8_t cMapperId;
    uint8_t cPRGBanks;
    uint8_t cCHRBanks;

    // mapper
    mapper* map;
} cartridge;

#ifdef __cplusplus
extern "C" {
#endif

void allocateMemory(memoryChunk* mem, uint32_t size);
void freeMemory(memoryChunk* mem);

void cartridgeInit(cartridge* cartridgeIn, const char* filename);

// writes to and reads from main bus
int cartCpuRead(cpu6502* cpu, cartridge* cart, uint16_t addr, uint8_t* data);
int cartCpuWrite(cpu6502* cpu, cartridge* cart, uint16_t addr, uint8_t val);

// writes to and reads from ppu bus
int cartPpuRead(ppu2C02* ppuIn, cartridge* cart, uint16_t addr, uint8_t* data);
int cartPpUWrite(ppu2C02* ppuIn, cartridge* cart, uint16_t addr, uint8_t val);

#ifdef __cplusplus
}
#endif
#endif //HHUOS_CARTRIDGE_H
