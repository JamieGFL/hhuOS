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
        // 0x8000 - 0xFFFF is mapped to PRG ROM
        // If there is only one bank, mirror it
        // If there are two banks, mask the address to 0x7FFF
        // If there are more than two banks, mask the address to 0x3FFF
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
int ppuMapRead(uint16_t addr, uint32_t* mapped_addr){
    // addr >= 0x0000 && addr <= 0x1FFF
    if(addr <= 0x1FFF){
        *mapped_addr = addr;
        return 1;
    }
    return 0;
}

int ppuMapWrite(){
    return 0;
}