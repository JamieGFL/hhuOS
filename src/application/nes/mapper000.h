#ifndef HHUOS_MAPPER000_H
#define HHUOS_MAPPER000_H
#include <stdint.h>
#include "mapper.h"
#include "ppu2C02.h"

#ifdef __cplusplus
extern "C" {
#endif

void mapper000Init(mapper* m, uint8_t prgBanks, uint8_t chrBanks);

int cpuMapRead(mapper* m, uint16_t addr, uint32_t* mapped_addr);
int cpuMapWrite(mapper* m, uint16_t addr, uint32_t* mapped_addr);
int ppuMapRead(mapper* m, ppu2C02* ppu, uint16_t addr, uint32_t* mapped_addr);
int ppuMapWrite(mapper* m, ppu2C02* ppu, uint16_t addr, uint32_t* mapped_addr);

#ifdef __cplusplus
}
#endif

#endif //HHUOS_MAPPER000_H
