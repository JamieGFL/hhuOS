#include "mapper000.h"

void mapper000Init(mapper* m, uint8_t prgBanks, uint8_t chrBanks){
    m->prgBanks = prgBanks;
    m->chrBanks = chrBanks;
    m->cpuMapRead = cpuMapRead;
    m->cpuMapWrite = cpuMapWrite;
    m->ppuMapRead = ppuMapRead;
    m->ppuMapWrite = ppuMapWrite;
}

int cpuMapRead(mapper* m, uint16_t addr, uint32_t* mapped_addr){
    // addr >= 0x8000 && addr <= 0xFFFF
    if(addr >= 0x8000){
        // if there is only 1 bank (16KB), 0x8000 - 0xBFFF is mapped to the 1 bank of PRG ROM,
        // and it is mirrored to 0xC000 - 0xFFFF
        // if there are 2 banks (32KB), 0x8000 - 0xFFFF is mapped to the 2 banks of PRG ROM
        *mapped_addr = addr & (m->prgBanks > 1 ? 0x7FFF : 0x3FFF);
        return 1;
    }
    return 0;
}

int cpuMapWrite(mapper* m, uint16_t addr, uint32_t* mapped_addr){
    // addr >= 0x8000 && addr <= 0xFFFF
    if(addr >= 0x8000){
        *mapped_addr = addr & (m->prgBanks > 1 ? 0x7FFF : 0x3FFF);
        return 1;
    }
    return 0;
}

int ppuMapRead(mapper* m, uint16_t addr, uint32_t* mapped_addr){
    // addr >= 0x0000 && addr <= 0x1FFF
    if(addr <= 0x1FFF){
        *mapped_addr = addr;
        return 1;
    }
    return 0;
}

int ppuMapWrite(mapper* m, uint16_t addr, uint32_t* mapped_addr){
    // addr >= 0x0000 && addr <= 0x1FFF
    if(addr <= 0x1FFF){
        if(m->chrBanks == 0){
            *mapped_addr = addr;
            return 1;
        }
    }
    return 0;
}