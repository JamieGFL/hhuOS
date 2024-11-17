#ifndef BUS_INTERFACE_H
#define BUS_INTERFACE_H



typedef struct bus {

    // Write to memory
    void (*bWrite)(struct bus* b, uint16_t addr, uint8_t val);
    // Read from memory
    uint8_t (*bRead)(struct bus* b, uint16_t addr, int readOnly);

    // 2KB RAM
    uint8_t ram[64 * 1024];
} bus;

void bWrite(bus* b, uint16_t addr, uint8_t val);
uint8_t bRead(bus* b, uint16_t addr, int readOnly);

#endif // BUS_INTERFACE_H