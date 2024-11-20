#ifndef HHUOS_PPU2C02_H
#define HHUOS_PPU2C02_H
#include "lib/cpu6502/cpu6502.h"
#include "cartridge.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct cartridge cartridge;

typedef struct pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} pixel;


typedef struct image {
    int32_t width;
    int32_t height;
    pixel* data; // Flat array of pixels
} image;

typedef struct ppu2C02 {
    cartridge* cart;

    // memory
    uint8_t nametable[2][1024];
    uint8_t palette[32];
    uint8_t patternTable[2][4096];

    // image
    pixel paletteScreen[64];
    image* screen;
    image* image_nametable[2];
    image* image_patternTable[2];
    int16_t scanline;
    int16_t cycle;
    bool frameComplete;

} ppu2C02;




#ifdef __cplusplus
extern "C" {
#endif

void ppuInit(ppu2C02* ppuIn);
void ppuDestroy(ppu2C02* ppuIn);

// writes to and reads from main bus
uint8_t ppuCpuRead(cpu6502* cpu, ppu2C02* ppuIn, uint16_t addr, int readOnly);
void ppuCpuWrite(cpu6502* cpu, ppu2C02* ppuIn, uint16_t addr, uint8_t val);

// writes to and reads from ppu bus
uint8_t ppuRead(ppu2C02* ppuIn, uint16_t addr, int readOnly);
void ppuWrite(ppu2C02* ppuIn, uint16_t addr, uint8_t val);


void connectCartridge(ppu2C02* ppuIn, cartridge* cart);
void ppuClock(ppu2C02* ppuIn);


// Image functions

static inline pixel createPixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
static inline int pixelEquals(pixel p1, pixel p2);

static inline image* createImage(int32_t width, int32_t height);
void freeImage(image* img);

static inline int setImagePixel(image* img, int32_t x, int32_t y, pixel p);
static inline pixel getImagePixel(image* img, int32_t x, int32_t y);

// Debugging

image* getPatternTableImage(ppu2C02* ppuIn, int index, uint8_t palette);

#ifdef __cplusplus
}
#endif

#endif