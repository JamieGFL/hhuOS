#include "ppu2C02.h"

static pixel createPixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
static int pixelEquals(pixel p1, pixel p2);

static image* createImage(int32_t width, int32_t height);
void freeImage(image* img);

static int setImagePixel(image* img, int32_t x, int32_t y, uint8_t p);
uint8_t getImagePixel(image* img, int32_t x, int32_t y);

static uint8_t getColorFromPalette(ppu2C02 *ppuIn, uint8_t palette, uint8_t pixel);

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

    ppuIn->nmi = false;

    ppuIn->adress_latch = 0x00;
    ppuIn->ppu_data_buffer = 0x00;
    ppuIn->vRamAddr.reg = 0x0000;
    ppuIn->tRamAddr.reg = 0x0000;

    ppuIn->bgNextTileId = 0x00;
    ppuIn->bgNextTileAttrib = 0x00;
    ppuIn->bgNextTileLsb = 0x00;
    ppuIn->bgNextTileMsb = 0x00;

    ppuIn->bgShifterPatternLo = 0x0000;
    ppuIn->bgShifterPatternHi = 0x0000;
    ppuIn->bgShifterAttribLo = 0x0000;
    ppuIn->bgShifterAttribHi = 0x0000;
}

void ppuDestroy(ppu2C02* ppuIn){
    freeImage(ppuIn->screen);
    freeImage(ppuIn->image_nametable[0]);
    freeImage(ppuIn->image_nametable[1]);
    freeImage(ppuIn->image_patternTable[0]);
    freeImage(ppuIn->image_patternTable[1]);
}

uint8_t ppuCpuRead(ppu2C02* ppuIn, uint16_t addr, int readOnly){
    uint8_t data = 0x00;

    switch(addr){
        case 0x0000: // Control
            break;
        case 0x0001: // Mask
            break;
        case 0x0002: // Status
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
            ppuIn->ppu_data_buffer = ppuRead(ppuIn, ppuIn->vRamAddr.reg);

            if(ppuIn->vRamAddr.reg >= 0x3F00) {
                data = ppuIn->ppu_data_buffer; }
            ppuIn->vRamAddr.reg += (ppuIn->control.increment_mode ? 32 : 1);
            break;
    }

    return data;
}

void ppuCpuWrite(ppu2C02* ppuIn, uint16_t addr, uint8_t val){

    switch(addr){
        case 0x0000: // Control
            ppuIn->control.reg = val;
            ppuIn->tRamAddr.nametable_x = ppuIn->control.nametable_x;
            ppuIn->tRamAddr.nametable_y = ppuIn->control.nametable_y;
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
            if(ppuIn->adress_latch == 0){
                ppuIn->fineX = val & 0x07;
                ppuIn->tRamAddr.coarse_x = val >> 3;
                ppuIn->adress_latch = 1;
            }
            else{
                ppuIn->tRamAddr.fine_y = val & 0x07;
                ppuIn->tRamAddr.coarse_y = val >> 3;
                ppuIn->adress_latch = 0;
            }
            break;
        case 0x0006: // PPU Address
            if(ppuIn->adress_latch == 0){
                ppuIn->tRamAddr.reg = (val << 8) | (ppuIn->tRamAddr.reg & 0x00FF);
                ppuIn->adress_latch = 1;

            }
            else{
                ppuIn->tRamAddr.reg = (ppuIn->tRamAddr.reg & 0xFF00) | val;
                ppuIn->vRamAddr = ppuIn->tRamAddr;
                ppuIn->adress_latch = 0;

            }
            break;
        case 0x0007: // PPU Data
            ppuWrite(ppuIn, ppuIn->vRamAddr.reg, val);
            ppuIn->vRamAddr.reg += (ppuIn->control.increment_mode ? 32 : 1);
            break;
    }
}

uint8_t ppuRead(ppu2C02* ppuIn, uint16_t addr){
    uint8_t data = 0x00;
    addr &= 0x3FFF;

    if(cartPpuRead(ppuIn, ppuIn->cart, addr, &data)){
        // do nothing
    }
    // addr >= 0x0000 && addr <= 0x1FFF
    else if(addr <= 0x1FFF){
        data = ppuIn->patternTable[(addr & 0x1000) >> 12][addr & 0x0FFF];
    }
    // nametable mirroring
    else if(addr >= 0x2000 && addr <= 0x3EFF){
        addr &= 0x0FFF;
        // vertical
        if(ppuIn->cart->cMirror == VERTICAL){
            // addr >= 0x0000 && addr <= 0x1FFF
            if(addr <= 0x03FF){
                data = ppuIn->nametable[0][addr & 0x03FF];
            }
            else if(addr >= 0x0400 && addr <= 0x07FF){
                data = ppuIn->nametable[1][addr & 0x03FF];
            }
            else if(addr >= 0x0800 && addr <= 0x0BFF){
                data = ppuIn->nametable[0][addr & 0x03FF];
            }
            else if(addr >= 0x0C00 && addr <= 0x0FFF){
                data = ppuIn->nametable[1][addr & 0x03FF];
            }
        }
        else if(ppuIn->cart->cMirror == HORIZONTAL){
            // horizontal
            // addr >= 0x0000 && addr <= 0x03FF
            if(addr <= 0x03FF){
                data = ppuIn->nametable[0][addr & 0x03FF];
            }
            else if(addr >= 0x0400 && addr <= 0x07FF){
                data = ppuIn->nametable[0][addr & 0x03FF];
            }
            else if(addr >= 0x0800 && addr <= 0x0BFF){
                data = ppuIn->nametable[1][addr & 0x03FF];
            }
            else if(addr >= 0x0C00 && addr <= 0x0FFF){
                data = ppuIn->nametable[1][addr & 0x03FF];
            }
        }


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
    addr &= 0x3FFF;
    if(cartPpUWrite(ppuIn, ppuIn->cart, addr, val)){
        // do nothing
    }
    else if(addr >= 0x0000 && addr <= 0x1FFF){
        ppuIn->patternTable[(addr & 0x1000) >> 12][addr & 0x0FFF] = val;
    }
    // nametable mirroring
    else if(addr >= 0x2000 && addr <= 0x3EFF){
        addr &= 0x0FFF;
        // vertical
        if(ppuIn->cart->cMirror == VERTICAL){
            // addr >= 0x0000 && addr <= 0x03FF
            if(addr <= 0x03FF){
                ppuIn->nametable[0][addr & 0x03FF] = val;
            }
            else if(addr >= 0x0400 && addr <= 0x07FF){
                ppuIn->nametable[1][addr & 0x03FF] = val;
            }
            else if(addr >= 0x0800 && addr <= 0x0BFF){
                ppuIn->nametable[0][addr & 0x03FF] = val;
            }
            else if(addr >= 0x0C00 && addr <= 0x0FFF){
                ppuIn->nametable[1][addr & 0x03FF] = val;
            }
        }
        // horizontal
        else if(ppuIn->cart->cMirror == HORIZONTAL){
            if(addr >= 0x0000 && addr <= 0x03FF){
                ppuIn->nametable[0][addr & 0x03FF] = val;
            }
            else if(addr >= 0x0400 && addr <= 0x07FF){
                ppuIn->nametable[0][addr & 0x03FF] = val;
            }
            else if(addr >= 0x0800 && addr <= 0x0BFF){
                ppuIn->nametable[1][addr & 0x03FF] = val;
            }
            else if(addr >= 0x0C00 && addr <= 0x0FFF){
                ppuIn->nametable[1][addr & 0x03FF] = val;
            }
        }
    }
    else if(addr >= 0x3F00 && addr <= 0x3FFF){
        addr &= 0x001F;
        if(addr == 0x0010) addr = 0x0000;
        if(addr == 0x0014) addr = 0x0004;
        if(addr == 0x0018) addr = 0x0008;
        if(addr == 0x001C) addr = 0x000C;
        ppuIn->palette[addr] = val;
    }

}

void connectCartridge(ppu2C02* ppuIn, cartridge* cart){
    ppuIn->cart = cart;
}

void incrementScrollX(ppu2C02* ppu)
{
    // when rendering is enabled
    if (ppu->mask.render_background || ppu->mask.render_sprites)
    {
        // name table is 32x30 tiles, so wrap x scroll around
        // if we overflow the current name table
        if (ppu->vRamAddr.coarse_x == 31)
        {
            ppu->vRamAddr.coarse_x = 0;
            ppu->vRamAddr.nametable_x = ~ppu->vRamAddr.nametable_x;
        }
        else
        {
            ppu->vRamAddr.coarse_x++;
        }
    }
}

void incrementScrollY(ppu2C02* ppu){
    // when rendering is enabled
    if (ppu->mask.render_background || ppu->mask.render_sprites)
    {
        if (ppu->vRamAddr.fine_y < 7)
        {
            ppu->vRamAddr.fine_y++;
        }
        else
        {
            ppu->vRamAddr.fine_y = 0;

            // name table is 32x30 tiles, so wrap y scroll around
            // if we overflow the current name table
            if (ppu->vRamAddr.coarse_y == 29)
            {
                ppu->vRamAddr.coarse_y = 0;
                ppu->vRamAddr.nametable_y = ~ppu->vRamAddr.nametable_y;
            }
            else if (ppu->vRamAddr.coarse_y == 31)
            {
                ppu->vRamAddr.coarse_y = 0;
            }
            else
            {
                ppu->vRamAddr.coarse_y++;
            }
        }
    }
}

void resetAddressX(ppu2C02* ppu){
    // when rendering is enabled
    if (ppu->mask.render_background || ppu->mask.render_sprites)
    {
        ppu->vRamAddr.nametable_x = ppu->tRamAddr.nametable_x;
        ppu->vRamAddr.coarse_x = ppu->tRamAddr.coarse_x;
    }
}

void resetAddressY(ppu2C02* ppu){
    // when rendering is enabled
    if (ppu->mask.render_background || ppu->mask.render_sprites)
    {
        ppu->vRamAddr.fine_y = ppu->tRamAddr.fine_y;
        ppu->vRamAddr.nametable_y = ppu->tRamAddr.nametable_y;
        ppu->vRamAddr.coarse_y = ppu->tRamAddr.coarse_y;
    }
}

void loadBackgroundShifters(ppu2C02* ppu){
    ppu->bgShifterPatternLo = (ppu->bgShifterPatternLo & 0xFF00) | ppu->bgNextTileLsb;
    ppu->bgShifterPatternHi = (ppu->bgShifterPatternHi & 0xFF00) | ppu->bgNextTileMsb;

    ppu->bgShifterAttribLo = (ppu->bgShifterAttribLo & 0xFF00) | ((ppu->bgNextTileAttrib & 0x01) ? 0xFF : 0x00);
    ppu->bgShifterAttribHi = (ppu->bgShifterAttribHi & 0xFF00) | ((ppu->bgNextTileAttrib & 0x02) ? 0xFF : 0x00);
}

void updateShifters(ppu2C02* ppu){
    if(ppu->mask.render_background){
        ppu->bgShifterPatternLo <<= 1;
        ppu->bgShifterPatternHi <<= 1;

        ppu->bgShifterAttribLo <<= 1;
        ppu->bgShifterAttribHi <<= 1;
    }
}

void ppuClock(ppu2C02* ppu) {

    if(ppu->scanline >= -1 && ppu->scanline < 240)
    {
        if (ppu->scanline == -1 && ppu->cycle == 1) {
            ppu->status.vertical_blank = 0;
        }

        if((ppu->cycle >= 2 && ppu->cycle < 258) || (ppu->cycle >= 321 && ppu->cycle < 338))
        {
            updateShifters(ppu);

            switch ((ppu->cycle - 1) % 8){
                case 0:
                    loadBackgroundShifters(ppu);
                    ppu->bgNextTileId = ppuRead(ppu, 0x2000 | (ppu->vRamAddr.reg & 0x0FFF));
                    break;
                case 2:
                    ppu->bgNextTileAttrib = ppuRead(ppu, 0x23C0 | (ppu->vRamAddr.nametable_y << 11)
                                                         | (ppu->vRamAddr.nametable_x << 10)
                                                         | ((ppu->vRamAddr.coarse_y >> 2) << 3)
                                                         | (ppu->vRamAddr.coarse_x >> 2));
                    if(ppu->vRamAddr.coarse_y & 0x02) ppu->bgNextTileAttrib >>= 4;
                    if(ppu->vRamAddr.coarse_x & 0x02) ppu->bgNextTileAttrib >>= 2;
                    ppu->bgNextTileAttrib &= 0x03;
                    break;
                case 4:
                    ppu->bgNextTileLsb = ppuRead(ppu, (ppu->control.pattern_background << 12)
                                                    + ((uint16_t)ppu->bgNextTileId << 4)
                                                    + (ppu->vRamAddr.fine_y) + 0);
                    break;
                case 6:
                    ppu->bgNextTileMsb = ppuRead(ppu, (ppu->control.pattern_background << 12)
                                                    + ((uint16_t)ppu->bgNextTileId << 4)
                                                    + (ppu->vRamAddr.fine_y) + 8);
                    break;
                case 7:
                    incrementScrollX(ppu);
                    break;
            }
        }

        if(ppu->cycle == 256)
        {
            incrementScrollY(ppu);
        }
        if(ppu->cycle == 257){
            resetAddressX(ppu);
        }
        if(ppu->scanline == -1 && ppu->cycle >= 280 && ppu->cycle < 305){
            resetAddressY(ppu);
        }
        if(ppu->scanline == 240){
            // do nothing
        }
    }



    // Set vertical blank flag when end of scanline is reached
    if(ppu->scanline == 241 && ppu->cycle == 1) {
        ppu->status.vertical_blank = 1;
        if(ppu->control.enable_nmi) {
            ppu->nmi = true;
        }
    }

    uint8_t bgPixel = 0x00;
    uint8_t bgPalette = 0x00;

    if(ppu->mask.render_background){
        uint16_t bitMux = 0x8000 >> ppu->fineX;
        uint8_t p0Pixel = (ppu->bgShifterPatternLo & bitMux) > 0;
        uint8_t p1Pixel = (ppu->bgShifterPatternHi & bitMux) > 0;
        bgPixel = (p1Pixel << 1) | p0Pixel;

        uint8_t bgPal0 = (ppu->bgShifterAttribLo & bitMux) > 0;
        uint8_t bgPal1 = (ppu->bgShifterAttribHi & bitMux) > 0;
        bgPalette = (bgPal1 << 1) | bgPal0;
    }

    uint8_t currentPixel = getColorFromPalette(ppu, bgPalette, bgPixel);
    setImagePixel(ppu->screen, ppu->cycle - 1, ppu->scanline, currentPixel);

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

static pixel createPixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a){
    pixel p = {r, g, b, a};
    return p;
}

static image* createImage(int32_t width, int32_t height){
    image* img = (image*)malloc(sizeof(image));
    if(img)
    {
        img->width = width;
        img->height = height;
        img->data = (uint8_t *) calloc(width * height, sizeof(pixel));
    }
    return img;
}
void freeImage(image* img){
    if(img) {
        free(img->data);
        free(img);
    }
}

static int setImagePixel(image* img, int32_t x, int32_t y, uint8_t p){
    if (x >= 0 && x < img->width && y >= 0 && y < img->height) {
        img->data[y * img->width + x] = p;
        return 1; // Success
    }
    return 0; // Out of bounds
}

uint8_t getImagePixel(image* img, int32_t x, int32_t y){
    if (x >= 0 && x < img->width && y >= 0 && y < img->height) {
        return img->data[y * img->width + x];
    }
    return 0; // Out of bounds
}

// Debugging

static uint8_t getColorFromPalette(ppu2C02 *ppuIn, uint8_t palette, uint8_t pixel) {
    return ppuRead(ppuIn,0x3F00 + (palette << 2) + pixel) & 0x3F;
}
image* getScreenImage(ppu2C02* ppuIn);
image* getNametableImage(ppu2C02* ppuIn, int index);

image* getPatternTableImage(ppu2C02* ppuIn, int index, uint8_t palette){
    for (int yTile = 0; yTile < 16; ++yTile) {
        for (int xTile = 0; xTile < 16; ++xTile) {
            uint16_t offset = yTile * 256 + xTile * 16;

            for (int row = 0; row < 8; ++row) {
                uint8_t lsb = ppuRead(ppuIn, index * 0x1000 + offset + row + 0x0000); // least significant bit, 0x1000 = 4 KB
                uint8_t msb = ppuRead(ppuIn, index * 0x1000 + offset + row + 0x0008); // most significant bit

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

