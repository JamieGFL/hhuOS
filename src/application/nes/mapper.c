#include "mapper.h"
#include <stdio.h>

void mapperInit(mapper* m, uint8_t prgBanks, uint8_t chrBanks){
    m->prgBanks = prgBanks;
    m->chrBanks = chrBanks;
    m->cpuMapRead = NULL;
    m->cpuMapWrite = NULL;
    m->ppuMapRead = NULL;
    m->ppuMapWrite = NULL;
}