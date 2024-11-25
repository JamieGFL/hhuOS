#include "cartridge.h"
#include "mapper000.h"
#include <stdlib.h>
#include <stdio.h>

static mapper map;

void allocateMemory(memoryChunk* mem, uint32_t size){
    mem->data = (uint8_t*)malloc(size);
    mem->size = size;
}
void freeMemory(memoryChunk* mem){
    free(mem->data);
    mem->data = NULL;
    mem->size = 0;
}

void cartridgeInit(cartridge* cartridgeIn, const char* filename){
    struct header {
        char name[4];
        uint8_t prg_rom_chunks;
        uint8_t chr_rom_chunks;
        uint8_t mapper1;
        uint8_t mapper2;
        uint8_t prg_ram_size;
        uint8_t tv_system1;
        uint8_t tv_system2;
        char unused[5];
    } header;

    FILE* file = fopen(filename, "rb");
    if(!file){
        printf("Error: Could not open file %s\n", filename);
        exit(1);
    }

    fread(&header, sizeof(header), 1, file);

    // next 512 bytes used for training information > not needed
    if(header.mapper1 & 0x04){
        fseek(file, 512, SEEK_CUR);
    }

    cartridgeIn->cMirror = HORIZONTAL;

    cartridgeIn->cMapperId = ((header.mapper2 >> 4) << 4) | (header.mapper1 >> 4);
    cartridgeIn->cMirror = (header.mapper1 & 0x01) ? VERTICAL : HORIZONTAL;

    uint8_t fileType = 1;
    if(fileType == 0){
    }
    else if(fileType == 1){
        cartridgeIn->map = &map;

        cartridgeIn->cPRGBanks = header.prg_rom_chunks;
        cartridgeIn->cCHRBanks = header.chr_rom_chunks;

        allocateMemory(&cartridgeIn->prg, cartridgeIn->cPRGBanks * 16384);
        allocateMemory(&cartridgeIn->chr, cartridgeIn->cCHRBanks * 8192);

        fread(cartridgeIn->prg.data, cartridgeIn->prg.size, 1, file);
        fread(cartridgeIn->chr.data, cartridgeIn->chr.size, 1, file);
    }
    else if(fileType == 2){
    }
    else if(fileType == 3){
    }

    // load mapper
    switch(cartridgeIn->cMapperId){
        case 0: {
            mapperInit(cartridgeIn->map, cartridgeIn->cPRGBanks, cartridgeIn->cCHRBanks);
            break;
        }
    }

    fclose(file);
}

int cartCpuRead(cpu6502* cpu, cartridge* cart, uint16_t addr, uint8_t* data){
    uint32_t mappedAddr = 0;
    if(cpuMapRead(cart->map, addr, &mappedAddr)){
        *data = cart->prg.data[mappedAddr];
        return 1;
    }
    else{
        return 0;
    }
}

int cartCpuWrite(cpu6502* cpu, cartridge* cart, uint16_t addr, uint8_t val){
    uint32_t mappedAddr = 0;
    if(cpuMapWrite(cart->map, addr, &mappedAddr)){
        cart->prg.data[mappedAddr] = val;
        return 1;
    }
    else{
        return 0;
    }
}

int cartPpuRead(ppu2C02* ppuIn, cartridge* cart, uint16_t addr, uint8_t* data){
    uint32_t mappedAddr = 0;
    if(ppuMapRead(cart->map, ppuIn, addr, &mappedAddr)){
        *data = cart->chr.data[mappedAddr];
        return 1;
    }
    else{
        return 0;
    }
}

int cartPpUWrite(ppu2C02* ppuIn, cartridge* cart, uint16_t addr, uint8_t val){
    uint32_t mappedAddr = 0;
    if(ppuMapWrite(cart->map, ppuIn, addr, &mappedAddr)){
        cart->chr.data[mappedAddr] = val;
        return 1;
    }
    else{
        return 0;
    }
}