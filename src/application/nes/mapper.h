#ifndef HHUOS_MAPPER_H
#define HHUOS_MAPPER_H
#include <stdint.h>

typedef struct mapper {
    uint8_t prgBanks;
    uint8_t chrBanks;
    int (*cpuMapRead)(struct mapper* m, uint16_t addr, uint32_t* mapped_addr);
    int (*cpuMapWrite)(struct mapper* m, uint16_t addr, uint32_t* mapped_addr);
    int (*ppuMapRead)(struct mapper* m, uint16_t addr, uint32_t* mapped_addr);
    int (*ppuMapWrite)(struct mapper* m, uint16_t addr, uint32_t* mapped_addr);
} mapper;

#ifdef __cplusplus
extern "C" {
#endif

void mapperInit(mapper* m, uint8_t prgBanks, uint8_t chrBanks);


#ifdef __cplusplus
}
#endif
#endif //HHUOS_MAPPER_H
