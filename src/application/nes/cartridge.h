#ifndef HHUOS_CARTRIDGE_H
#define HHUOS_CARTRIDGE_H
#include "mapper.h"

typedef struct ppu2C02 ppu2C02;
typedef struct mapper mapper;
typedef struct {
    uint8_t *data;
    uint32_t size;
} memoryChunk;

 typedef enum MIRROR {
    HORIZONTAL,
    VERTICAL,
    ONESCREEN_LO,
    ONESCREEN_HI,
}mirror;

typedef struct cartridge {
    // memory
    memoryChunk prg;
    memoryChunk chr;

    uint8_t mapperID;
    // mirror
    mirror mirrorMode;
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

// CPU reads from cartridge
int cartCpuRead(cartridge* cart, uint16_t addr, uint8_t* data);

// CPU writes to cartridge
int cartCpuWrite(cartridge* cart, uint16_t addr, uint8_t val);

// PPU reads from cartridge
int cartPpuRead(cartridge* cart, uint16_t addr, uint8_t* data);

// PPU writes to cartridge
int cartPpUWrite(cartridge* cart, uint16_t addr, uint8_t val);

#ifdef __cplusplus
}
#endif
#endif //HHUOS_CARTRIDGE_H
