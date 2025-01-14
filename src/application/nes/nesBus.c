#include "nesBus.h"

static bus_nes* nesBus;

void nesBusWrite(bus* b, uint16_t addr, uint8_t val);
uint8_t nesBusRead(bus* b, uint16_t addr, int readOnly);

void bInit(bus_nes* b, bus* bus, cpu6502* cpu, ppu2C02* ppu) {
    b->busBase = bus;

    // set function pointers
    b->busBase->bWrite = nesBusWrite;
    b->busBase->bRead = nesBusRead;

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

void setBus(bus_nes *b) {
    nesBus = b;
}

void nesBusWrite(bus* b, uint16_t addr, uint8_t val) {
        // Cartridge
        if(cartCpuWrite(nesBus->cart, addr, val)){
            // do nothing
        }
        // CPU RAM
        else if(addr <= 0x1FFF){
            b->ram[addr & 0x07FF] = val;
        }
        // PPU registers
        else if(addr >= 0x2000 && addr <= 0x3FFF){
            ppuCpuWrite(nesBus->ppu, addr & 0x0007, val);
        }
        // DMA
        else if(addr == 0x4014){
            nesBus->dmaPage = val;
            nesBus->dmaAddr = 0x00;
            nesBus->dmaTransfer = true;
        }
        // Controller
        else if(addr >= 0x4016 && addr <= 0x4017){
            nesBus->controllerState[addr & 0x0001] = nesBus->controller[addr & 0x0001];
        }
}

uint8_t nesBusRead(bus* b, uint16_t addr, int readOnly) {
    uint8_t data = 0x00;

    if(cartCpuRead(nesBus->cart, addr, &data)){
        // do nothing
    }
    // CPU RAM
    else if(addr <= 0x1FFF){
        data = b->ram[addr & 0x07FF];
    }
    // PPU registers
    else if(addr >= 0x2000 && addr <= 0x3FFF){
        data = ppuCpuRead(nesBus->ppu, addr & 0x0007, readOnly);
    }
    // Controller
    else if(addr >= 0x4016 && addr <= 0x4017){
        data = (nesBus->controllerState[addr & 0x0001] & 0x80) > 0;
        nesBus->controllerState[addr & 0x0001] <<= 1;
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
                    b->dmaData = nesBusRead(b->busBase, (uint16_t) (b->dmaPage << 8 | b->dmaAddr), false);
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