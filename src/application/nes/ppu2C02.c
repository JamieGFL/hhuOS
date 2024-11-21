#include "ppu2C02.h"
#include "cartridge.h"

static inline pixel createPixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
static inline int pixelEquals(pixel p1, pixel p2);

static inline image* createImage(int32_t width, int32_t height);
void freeImage(image* img);

static inline int setImagePixel(image* img, int32_t x, int32_t y, pixel p);
static inline pixel getImagePixel(image* img, int32_t x, int32_t y);

void ppuInit(ppu2C02* ppuIn){
    ppuIn->paletteScreen[0x00] = createPixel(84, 84, 84, 255);
    ppuIn->paletteScreen[0x01] = createPixel(0, 30, 116, 255);
    ppuIn->paletteScreen[0x02] = createPixel(8, 16, 144, 255);
    ppuIn->paletteScreen[0x03] = createPixel(48, 0, 136, 255);
    ppuIn->paletteScreen[0x04] = createPixel(68, 0, 100, 255);
    ppuIn->paletteScreen[0x05] = createPixel(92, 0, 48, 255);
    ppuIn->paletteScreen[0x06] = createPixel(84, 4, 0, 255);
    ppuIn->paletteScreen[0x07] = createPixel(60, 24, 0, 255);
    ppuIn->paletteScreen[0x08] = createPixel(32, 42, 0, 255);
    ppuIn->paletteScreen[0x09] = createPixel(8, 58, 0, 255);
    ppuIn->paletteScreen[0x0A] = createPixel(0, 64, 0, 255);
    ppuIn->paletteScreen[0x0B] = createPixel(0, 60, 0, 255);
    ppuIn->paletteScreen[0x0C] = createPixel(0, 50, 60, 255);
    ppuIn->paletteScreen[0x0D] = createPixel(0, 0, 0, 255);
    ppuIn->paletteScreen[0x0E] = createPixel(0, 0, 0, 255);
    ppuIn->paletteScreen[0x0F] = createPixel(0, 0, 0, 255);

    ppuIn->paletteScreen[0x10] = createPixel(152, 150, 152, 255);
    ppuIn->paletteScreen[0x11] = createPixel(8, 76, 196, 255);
    ppuIn->paletteScreen[0x12] = createPixel(48, 50, 236, 255);
    ppuIn->paletteScreen[0x13] = createPixel(92, 30, 228, 255);
    ppuIn->paletteScreen[0x14] = createPixel(136, 20, 176, 255);
    ppuIn->paletteScreen[0x15] = createPixel(160, 20, 100, 255);
    ppuIn->paletteScreen[0x16] = createPixel(152, 34, 32, 255);
    ppuIn->paletteScreen[0x17] = createPixel(120, 60, 0, 255);
    ppuIn->paletteScreen[0x18] = createPixel(84, 90, 0, 255);
    ppuIn->paletteScreen[0x19] = createPixel(40, 114, 0, 255);
    ppuIn->paletteScreen[0x1A] = createPixel(8, 124, 0, 255);
    ppuIn->paletteScreen[0x1B] = createPixel(0, 118, 40, 255);
    ppuIn->paletteScreen[0x1C] = createPixel(0, 102, 120, 255);
    ppuIn->paletteScreen[0x1D] = createPixel(0, 0, 0, 255);
    ppuIn->paletteScreen[0x1E] = createPixel(0, 0, 0, 255);
    ppuIn->paletteScreen[0x1F] = createPixel(0, 0, 0, 255);

    ppuIn->paletteScreen[0x20] = createPixel(236, 238, 236, 255);
    ppuIn->paletteScreen[0x21] = createPixel(76, 154, 236, 255);
    ppuIn->paletteScreen[0x22] = createPixel(120, 124, 236, 255);
    ppuIn->paletteScreen[0x23] = createPixel(176, 98, 236, 255);
    ppuIn->paletteScreen[0x24] = createPixel(228, 84, 236, 255);
    ppuIn->paletteScreen[0x25] = createPixel(236, 88, 180, 255);
    ppuIn->paletteScreen[0x26] = createPixel(236, 106, 100, 255);
    ppuIn->paletteScreen[0x27] = createPixel(212, 136, 32, 255);
    ppuIn->paletteScreen[0x28] = createPixel(160, 170, 0, 255);
    ppuIn->paletteScreen[0x29] = createPixel(116, 196, 0, 255);
    ppuIn->paletteScreen[0x2A] = createPixel(76, 208, 32, 255);
    ppuIn->paletteScreen[0x2B] = createPixel(56, 204, 108, 255);
    ppuIn->paletteScreen[0x2C] = createPixel(56, 180, 204, 255);
    ppuIn->paletteScreen[0x2D] = createPixel(60, 60, 60, 255);
    ppuIn->paletteScreen[0x2E] = createPixel(0, 0, 0, 255);
    ppuIn->paletteScreen[0x2F] = createPixel(0, 0, 0, 255);

    ppuIn->paletteScreen[0x30] = createPixel(236, 238, 236, 255);
    ppuIn->paletteScreen[0x31] = createPixel(168, 204, 236, 255);
    ppuIn->paletteScreen[0x32] = createPixel(188, 188, 236, 255);
    ppuIn->paletteScreen[0x33] = createPixel(212, 178, 236, 255);
    ppuIn->paletteScreen[0x34] = createPixel(236, 174, 236, 255);
    ppuIn->paletteScreen[0x35] = createPixel(236, 174, 212, 255);
    ppuIn->paletteScreen[0x36] = createPixel(236, 180, 176, 255);
    ppuIn->paletteScreen[0x37] = createPixel(228, 196, 144, 255);
    ppuIn->paletteScreen[0x38] = createPixel(204, 210, 120, 255);
    ppuIn->paletteScreen[0x39] = createPixel(180, 222, 120, 255);
    ppuIn->paletteScreen[0x3A] = createPixel(168, 226, 144, 255);
    ppuIn->paletteScreen[0x3B] = createPixel(152, 226, 180, 255);
    ppuIn->paletteScreen[0x3C] = createPixel(160, 214, 228, 255);
    ppuIn->paletteScreen[0x3D] = createPixel(160, 162, 160, 255);
    ppuIn->paletteScreen[0x3E] = createPixel(0, 0, 0, 255);
    ppuIn->paletteScreen[0x3F] = createPixel(0, 0, 0, 255);

    ppuIn->screen = createImage(256, 240);
    ppuIn->image_nametable[0] = createImage(256, 240);
    ppuIn->image_nametable[1] = createImage(256, 240);
    ppuIn->image_patternTable[0] = createImage(128, 128);
    ppuIn->image_patternTable[1] = createImage(128, 128);

    ppuIn->scanline = 0;
    ppuIn->cycle = 0;
    ppuIn->frameComplete = false;

    ppuIn->adress_latch = 0x00;
    ppuIn->ppu_data_buffer = 0x00;
    ppuIn->ppu_address = 0x0000;

}

void ppuDestroy(ppu2C02* ppuIn){
    freeImage(ppuIn->screen);
    freeImage(ppuIn->image_nametable[0]);
    freeImage(ppuIn->image_nametable[1]);
    freeImage(ppuIn->image_patternTable[0]);
    freeImage(ppuIn->image_patternTable[1]);
}

uint8_t ppuCpuRead(cpu6502* cpu, ppu2C02* ppuIn, uint16_t addr, int readOnly){
    uint8_t data = 0x00;

    switch(addr){
        case 0x0000: // Control
            break;
        case 0x0001: // Mask
            break;
        case 0x0002: // Status
            ppuIn->status.vertical_blank = 1;
            data = (ppuIn->status.reg & 0xE0) | (ppuIn->ppu_data_buffer & 0x1F);
            ppuIn->status.vertical_blank = 0;
            ppuIn->adress_latch = 0;
            break;
        case 0x0003: // OAM Address
            break;
        case 0x0004: // OAM Data
            break;
        case 0x0005: // Scroll
            break;
        case 0x0006: // PPU Address
            break;
        case 0x0007: // PPU Data
            data = ppuIn->ppu_data_buffer;
            ppuIn->ppu_data_buffer = ppuRead(ppuIn, ppuIn->ppu_address, readOnly);

            if(ppuIn->ppu_address > 0x3F00) data = ppuIn->ppu_data_buffer;
            break;
    }

    return data;
}

void ppuCpuWrite(cpu6502* cpu, ppu2C02* ppuIn, uint16_t addr, uint8_t val){

    switch(addr){
        case 0x0000: // Control
            ppuIn->control.reg = val;
            break;
        case 0x0001: // Mask
            ppuIn->mask.reg = val;
            break;
        case 0x0002: // Status
            break;
        case 0x0003: // OAM Address
            break;
        case 0x0004: // OAM Data
            break;
        case 0x0005: // Scroll
            break;
        case 0x0006: // PPU Address
            if(ppuIn->adress_latch == 0){
                ppuIn->ppu_address = val | (ppuIn->ppu_address & 0x00FF);
                ppuIn->adress_latch = 1;
            }
            else{
                ppuIn->ppu_address = (ppuIn->ppu_address & 0xFF00) | val;
                ppuIn->adress_latch = 0;
            }
            break;
        case 0x0007: // PPU Data
            ppuWrite(ppuIn, ppuIn->ppu_address, val);
            break;
    }
}

uint8_t ppuRead(ppu2C02* ppuIn, uint16_t addr, int readOnly){
    uint8_t data = 0x00;
    addr &= 0x3FFF;

    if(cartPpuRead(ppuIn, ppuIn->cart, addr, &data)){
        // do nothing
    }
    else if(addr >= 0x0000 && addr <= 0x1FFF){
        data = ppuIn->patternTable[(addr & 0x1000) >> 12][addr & 0x0FFF];
    }
    else if(addr >= 0x2000 && addr <= 0x3EFF){
    }
    else if(addr >= 0x3F00 && addr <= 0x3FFF){
        addr &= 0x001F;
        if(addr == 0x0010) addr = 0x0000;
        if(addr == 0x0014) addr = 0x0004;
        if(addr == 0x0018) addr = 0x0008;
        if(addr == 0x001C) addr = 0x000C;
        data = ppuIn->palette[addr];
    }

    return data;
}
void ppuWrite(ppu2C02* ppuIn, uint16_t addr, uint8_t val){

    if(cartPpUWrite(ppuIn, ppuIn->cart, addr, val)){
        // do nothing
    }
    else if(addr >= 0x0000 && addr <= 0x1FFF){
        ppuIn->patternTable[(addr & 0x1000) >> 12][addr & 0x0FFF] = val;
    }
    else if(addr >= 0x2000 && addr <= 0x3EFF){
    }
    else if(addr >= 0x3F00 && addr <= 0x3FFF){
        addr &= 0x001F;
        if(addr == 0x0010) addr = 0x0000;
        if(addr == 0x0014) addr = 0x0004;
        if(addr == 0x0018) addr = 0x0008;
        if(addr == 0x001C) addr = 0x000C;
        ppuIn->palette[addr] = val;
    }

    addr &= 0x3FFF;
}

void connectCartridge(ppu2C02* ppuIn, cartridge* cart){
    ppuIn->cart = cart;
}

void ppuClock(ppu2C02* ppu) {
    // Simulate rendering noise with black or white pixels
    pixel randomPixel = ppu->paletteScreen[rand() % 2 ? 0x3F : 0x30];
    setImagePixel(ppu->screen, ppu->cycle - 1, ppu->scanline, randomPixel);

    // Advance the renderer
    ppu->cycle++;
    if (ppu->cycle >= 341) {
        ppu->cycle = 0;
        ppu->scanline++;
        if (ppu->scanline >= 261) {
            ppu->scanline = -1;
            ppu->frameComplete = true;
        }
    }
}


// Image functions


static inline pixel createPixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a){
    pixel p = {r, g, b, a};
    return p;
}

static inline int pixelEquals(pixel p1, pixel p2){
    return p1.r == p2.r && p1.g == p2.g && p1.b == p2.b && p1.a == p2.a;
}

static inline image* createImage(int32_t width, int32_t height){
    image* img = (image*)malloc(sizeof(image));
    if(img)
    {
        img->width = width;
        img->height = height;
        img->data = (pixel *) calloc(width * height, sizeof(pixel));
    }
    return img;
}
void freeImage(image* img){
    if(img) {
        free(img->data);
        free(img);
    }
}

static inline int setImagePixel(image* img, int32_t x, int32_t y, pixel p){
    if (x >= 0 && x < img->width && y >= 0 && y < img->height) {
        img->data[y * img->width + x] = p;
        return 1; // Success
    }
    return 0; // Out of bounds
}

static inline pixel getImagePixel(image* img, int32_t x, int32_t y){
    if (x >= 0 && x < img->width && y >= 0 && y < img->height) {
        return img->data[y * img->width + x];
    }
    return createPixel(0, 0, 0, 0); // Out of bounds
}

// Debugging

pixel getColorFromPalette(ppu2C02 *ppuIn, uint8_t palette, uint8_t pixel) {
    return ppuIn->paletteScreen[ppuRead(ppuIn,0x3F00 + (palette << 2) + pixel, 0) & 0x3F];
}
image* getScreenImage(ppu2C02* ppuIn);
image* getNametableImage(ppu2C02* ppuIn, int index);

image* getPatternTableImage(ppu2C02* ppuIn, int index, uint8_t palette){
    for (int yTile = 0; yTile < 16; ++yTile) {
        for (int xTile = 0; xTile < 16; ++xTile) {
            uint16_t offset = yTile * 256 + xTile * 16;

            for (int row = 0; row < 8; ++row) {
                uint8_t lsb = ppuRead(ppuIn, index * 0x1000 + offset + row + 0, false); // least significant bit, 0x1000 = 4 KB
                uint8_t msb = ppuRead(ppuIn, index * 0x1000 + offset + row + 8, false); // most significant bit

                for (int col = 0; col < 8; ++col) {
                    uint8_t pixel = (lsb & 0x01) + (msb & 0x01);
                    lsb >>= 1;
                    msb >>= 1;

                    setImagePixel(ppuIn->image_patternTable[index], xTile * 8 + (7 - col)
                                  , yTile * 8 + row,
                                  getColorFromPalette(ppuIn, palette, pixel));
                }
            }
        }
    }

    return ppuIn->image_patternTable[index];
}

