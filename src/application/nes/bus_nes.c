#include "bus_nes.h"

static bus_nes* nesBus;

void nes_bus_Write(bus* b, uint16_t addr, uint8_t val);
uint8_t nes_bus_Read(bus* b, uint16_t addr, int readOnly);

void bInit(bus_nes* b, bus* bus, cpu6502* cpu, ppu2C02* ppu) {
    b->busBase = bus;

    // set function pointers
    b->busBase->bWrite = nes_bus_Write;
    b->busBase->bRead = nes_bus_Read;

    for (unsigned int i = 0; i < sizeof(b->busBase->ram); i++) {
        b->busBase->ram[i] = 0x00;
    }
    b->cpu = cpu;
    b->ppu = ppu;

    // dma
    b->dmaPage = 0x00;
    b->dmaAddr = 0x00;
    b->dmaData = 0x00;
    b->dmaTransfer = false;
    b->dmaDummy = true;

    connectBus(b->cpu, b->busBase);
}

void bDestroy(bus_nes* b) {
    free(b->cpu);
    ppuDestroy(b->ppu);
    free(b->ppu);
    free(b->cart);
    free(b->busBase);
    free(b);
}

void setBus(bus_nes *b) {
    nesBus = b;
}

void nes_bus_Write(bus* b, uint16_t addr, uint8_t val) {

        if(cartCpuWrite(nesBus->cart, addr, val)){
            // do nothing
        }
        else if(addr <= 0x1FFF){
            b->ram[addr & 0x07FF] = val;
        }
        else if(addr >= 0x2000 && addr <= 0x3FFF){
            ppuCpuWrite(nesBus->ppu, addr & 0x0007, val);
        }
        else if(addr == 0x4014){
            nesBus->dmaPage = val;
            nesBus->dmaAddr = 0x00;
            nesBus->dmaTransfer = true;
        }
        else if(addr >= 0x4016 && addr <= 0x4017){
            nesBus->controllerState[addr & 0x0001] = nesBus->controller[addr & 0x0001];
        }
}

uint8_t nes_bus_Read(bus* b, uint16_t addr, int readOnly) {
    uint8_t data = 0x00;

    if(cartCpuRead(nesBus->cart, addr, &data)){
        // do nothing
    }
    else if(addr <= 0x1FFF){
        data = b->ram[addr & 0x07FF];
    }
    else if(addr >= 0x2000 && addr <= 0x3FFF){
        data = ppuCpuRead(nesBus->ppu, addr & 0x0007, readOnly);
    }
    else if(addr >= 0x4016 && addr <= 0x4017){
        data = (nesBus->controllerState[addr & 0x0001] & 0x80) > 0;
        nesBus->controllerState[addr & 0x0001] <<= 1;
    }

    if(readOnly) {
        readOnly = 0;
    }

    return data;
}

void insertCartridge(bus_nes* b, cartridge* cart) {
    b->cart = cart;
    connectCartridge(b->ppu, b->cart);
}

void busReset(bus_nes* b) {
    cpuReset(b->cpu);
    ppuReset(b->ppu);
    b->clockCount = 0;
    b->dmaPage = 0x00;
    b->dmaAddr = 0x00;
    b->dmaData = 0x00;
    b->dmaTransfer = false;
    b->dmaDummy = true;

}

void busClock(bus_nes* b) {
    ppuClock(b->ppu);
    if (b->clockCount % 3 == 0) {
        if(b->dmaTransfer){
            if(b->dmaDummy){
                if(b->clockCount % 2 == 1){
                    b->dmaDummy = false;
                }
            }
            else{
                if(b->clockCount % 2 == 0){
                    b->dmaData = nes_bus_Read(b->busBase, (uint16_t)(b->dmaPage << 8 | b->dmaAddr), false);
                }
                else{
                    nesBus->ppu->oamPointer[b->dmaAddr] = b->dmaData;
                    b->dmaAddr++;
                    if(b->dmaAddr == 0x00){
                        b->dmaTransfer = false;
                        b->dmaDummy = true;
                    }
                }
            }
        }
        cpuClock(b->cpu);
    }
    if(b->ppu->nmi){
        b->ppu->nmi = false;
        nmi(b->cpu);
    }

    b->clockCount++;
}

image* getScreenImage(bus_nes* b){
    return b->ppu->screen;
}

bool frameComplete(bus_nes* b){
    return b->ppu->frameComplete;
}

void setFrameComplete(bus_nes* b, bool complete){
    b->ppu->frameComplete = complete;
}