#include "ppu2C02.h"
#include "cartridge.h"

void ppuInit(ppu2C02* ppuIn);

uint8_t ppuCpuRead(cpu6502* cpu, ppu2C02* ppuIn, uint16_t addr, int readOnly){
    uint8_t data = 0x00;

    switch(addr){
        case 0x0000: // Control
            break;
        case 0x0001: // Mask
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
            break;
        case 0x0007: // PPU Data
            break;
    }

    return data;
}

void ppuCpuWrite(cpu6502* cpu, ppu2C02* ppuIn, uint16_t addr, uint8_t val){

    switch(addr){
        case 0x0000: // Control
            break;
        case 0x0001: // Mask
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
            break;
        case 0x0007: // PPU Data
            break;
    }
}

uint8_t ppuRead(ppu2C02* ppuIn, uint16_t addr, int readOnly){
    uint8_t data = 0x00;
    addr &= 0x3FFF;

    if(cartPpuRead(ppuIn, ppuIn->cart, addr)){
        // do nothing
    }

    return data;
}
void ppuWrite(ppu2C02* ppuIn, uint16_t addr, uint8_t val){

    if(ppuIn->cart->cartPpUWrite(ppuIn, ppuIn->cart, addr, val)){
        // do nothing
    }

    addr &= 0x3FFF;
}

void connectCartridge(ppu2C02* ppuIn, cartridge* cart){
    ppuIn->cart = cart;
}