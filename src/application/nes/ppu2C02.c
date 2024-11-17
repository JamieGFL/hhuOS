#include "ppu2C02.h"

void ppuInit(ppu2C02* ppuIn);

uint8_t cpuRead(cpu6502* cpu, ppu2C02* ppuIn, uint16_t addr, int readOnly){

    return 0;
}

void cpuWrite(cpu6502* cpu, ppu2C02* ppuIn, uint16_t addr, uint8_t val){
    return;
}

uint8_t ppuRead(ppu2C02* ppuIn, uint16_t addr, int readOnly);
void ppuWrite(ppu2C02* ppuIn, uint16_t addr, uint8_t val);