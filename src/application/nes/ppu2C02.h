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

    // clock
    int16_t scanline;
    int16_t cycle;
    bool frameComplete;

    bool nmi;

    union
    {
        struct
        {
            uint8_t unused : 5;
            uint8_t sprite_overflow : 1;
            uint8_t sprite_zero_hit : 1;
            uint8_t vertical_blank : 1;
        };
        uint8_t reg;
    } status;

    union
    {
        struct
        {
            uint8_t grayscale : 1;
            uint8_t render_background_left : 1;
            uint8_t render_sprites_left : 1;
            uint8_t render_background : 1;
            uint8_t render_sprites : 1;
            uint8_t enhance_red : 1;
            uint8_t enhance_green : 1;
            uint8_t enhance_blue : 1;
        };
        uint8_t reg;
    } mask;

    union
    {
        struct
        {
            uint8_t nametable_x : 1;
            uint8_t nametable_y : 1;
            uint8_t increment_mode : 1;
            uint8_t pattern_sprite : 1;
            uint8_t pattern_background : 1;
            uint8_t sprite_size : 1;
            uint8_t slave_mode : 1;
            uint8_t enable_nmi : 1;
        };
        uint8_t reg;
    } control;

    uint8_t adress_latch;
    uint8_t ppu_data_buffer;
    uint16_t ppu_address;
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

static pixel createPixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
static int pixelEquals(pixel p1, pixel p2);

static image* createImage(int32_t width, int32_t height);
void freeImage(image* img);

static int setImagePixel(image* img, int32_t x, int32_t y, pixel p);
pixel getImagePixel(image* img, int32_t x, int32_t y);

// Debugging

image* getPatternTableImage(ppu2C02* ppuIn, int index, uint8_t palette);

#ifdef __cplusplus
}
#endif

#endif