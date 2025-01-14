#ifndef HHUOS_NESBUS_H
#define HHUOS_NESBUS_H

#include <stdint.h>
#include "lib/cpu6502/cpu6502.h"
#include "ppu2C02.h"
#include "cartridge.h"

typedef struct {
    // how many cycles have passed
    uint32_t clockCount;

    // components
    cpu6502* cpu;
    ppu2C02* ppu;
    cartridge* cart;

    // bus
    bus* busBase;

    // controller
    uint8_t controller[2];
    uint8_t controllerState[2];

    // dma
    uint8_t dmaPage;
    uint8_t dmaAddr;
    uint8_t dmaData;
    bool dmaTransfer;
    bool dmaDummy;

} bus_nes;

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the NES bus
void bInit(bus_nes* b, bus* bus, cpu6502* cpu, ppu2C02* ppu);

// Set static bus
void setBus(bus_nes *b);

// Write to memory through bus
void nesBusWrite(bus* b, uint16_t addr, uint8_t val);

// Read from memory through bus
uint8_t nesBusRead(bus* b, uint16_t addr, int readOnly);

// Insert cartridge into NES bus
void insertCartridge(bus_nes* b, cartridge* cart);

// Reset NES
void busReset(bus_nes* b);

// Clock the NES
void busClock(bus_nes* b);

// Utility

image* getScreenImage(bus_nes* b);
bool frameComplete(bus_nes* b);
void setFrameComplete(bus_nes* b, bool complete);
#ifdef __cplusplus
}
#endif


#endif //HHUOS_NESBUS_H
