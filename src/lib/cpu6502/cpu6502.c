#include "cpu6502.h"
#include "bus_interface.h"
#include "lib/libc/stdio.h"
#include "lib/libc/string.h"
#include <stddef.h>




// Lookup table for instructions in C
static instruction lookupTable [256]= {
        { "BRK", BRK, IMP, 7 },{ "ORA", ORA, IZX, 6 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 8 },{ "???", NOP, IMP, 3 },{ "ORA", ORA, ZP0, 3 },{ "ASL", ASL, ZP0, 5 },{ "???", XXX, IMP, 5 },{ "PHP", PHP, IMP, 3 },{ "ORA", ORA, IMM, 2 },{ "ASL", ASL, IMP, 2 },{ "???", XXX, IMP, 2 },{ "???", NOP, IMP, 4 },{ "ORA", ORA, ABS, 4 },{ "ASL", ASL, ABS, 6 },{ "???", XXX, IMP, 6 },
        { "BPL", BPL, REL, 2 },{ "ORA", ORA, IZY, 5 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 8 },{ "???", NOP, IMP, 4 },{ "ORA", ORA, ZPX, 4 },{ "ASL", ASL, ZPX, 6 },{ "???", XXX, IMP, 6 },{ "CLC", CLC, IMP, 2 },{ "ORA", ORA, ABY, 4 },{ "???", NOP, IMP, 2 },{ "???", XXX, IMP, 7 },{ "???", NOP, IMP, 4 },{ "ORA", ORA, ABX, 4 },{ "ASL", ASL, ABX, 7 },{ "???", XXX, IMP, 7 },
        { "JSR", JSR, ABS, 6 },{ "AND", AND, IZX, 6 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 8 },{ "BIT", BIT, ZP0, 3 },{ "AND", AND, ZP0, 3 },{ "ROL", ROL, ZP0, 5 },{ "???", XXX, IMP, 5 },{ "PLP", PLP, IMP, 4 },{ "AND", AND, IMM, 2 },{ "ROL", ROL, IMP, 2 },{ "???", XXX, IMP, 2 },{ "BIT", BIT, ABS, 4 },{ "AND", AND, ABS, 4 },{ "ROL", ROL, ABS, 6 },{ "???", XXX, IMP, 6 },
        { "BMI", BMI, REL, 2 },{ "AND", AND, IZY, 5 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 8 },{ "???", NOP, IMP, 4 },{ "AND", AND, ZPX, 4 },{ "ROL", ROL, ZPX, 6 },{ "???", XXX, IMP, 6 },{ "SEC", SEC, IMP, 2 },{ "AND", AND, ABY, 4 },{ "???", NOP, IMP, 2 },{ "???", XXX, IMP, 7 },{ "???", NOP, IMP, 4 },{ "AND", AND, ABX, 4 },{ "ROL", ROL, ABX, 7 },{ "???", XXX, IMP, 7 },
        { "RTI", RTI, IMP, 6 },{ "EOR", EOR, IZX, 6 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 8 },{ "???", NOP, IMP, 3 },{ "EOR", EOR, ZP0, 3 },{ "LSR", LSR, ZP0, 5 },{ "???", XXX, IMP, 5 },{ "PHA", PHA, IMP, 3 },{ "EOR", EOR, IMM, 2 },{ "LSR", LSR, IMP, 2 },{ "???", XXX, IMP, 2 },{ "JMP", JMP, ABS, 3 },{ "EOR", EOR, ABS, 4 },{ "LSR", LSR, ABS, 6 },{ "???", XXX, IMP, 6 },
        { "BVC", BVC, REL, 2 },{ "EOR", EOR, IZY, 5 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 8 },{ "???", NOP, IMP, 4 },{ "EOR", EOR, ZPX, 4 },{ "LSR", LSR, ZPX, 6 },{ "???", XXX, IMP, 6 },{ "CLI", CLI, IMP, 2 },{ "EOR", EOR, ABY, 4 },{ "???", NOP, IMP, 2 },{ "???", XXX, IMP, 7 },{ "???", NOP, IMP, 4 },{ "EOR", EOR, ABX, 4 },{ "LSR", LSR, ABX, 7 },{ "???", XXX, IMP, 7 },
        { "RTS", RTS, IMP, 6 },{ "ADC", ADC, IZX, 6 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 8 },{ "???", NOP, IMP, 3 },{ "ADC", ADC, ZP0, 3 },{ "ROR", ROR, ZP0, 5 },{ "???", XXX, IMP, 5 },{ "PLA", PLA, IMP, 4 },{ "ADC", ADC, IMM, 2 },{ "ROR", ROR, IMP, 2 },{ "???", XXX, IMP, 2 },{ "JMP", JMP, IND, 5 },{ "ADC", ADC, ABS, 4 },{ "ROR", ROR, ABS, 6 },{ "???", XXX, IMP, 6 },
        { "BVS", BVS, REL, 2 },{ "ADC", ADC, IZY, 5 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 8 },{ "???", NOP, IMP, 4 },{ "ADC", ADC, ZPX, 4 },{ "ROR", ROR, ZPX, 6 },{ "???", XXX, IMP, 6 },{ "SEI", SEI, IMP, 2 },{ "ADC", ADC, ABY, 4 },{ "???", NOP, IMP, 2 },{ "???", XXX, IMP, 7 },{ "???", NOP, IMP, 4 },{ "ADC", ADC, ABX, 4 },{ "ROR", ROR, ABX, 7 },{ "???", XXX, IMP, 7 },
        { "???", NOP, IMP, 2 },{ "STA", STA, IZX, 6 },{ "???", NOP, IMP, 2 },{ "???", XXX, IMP, 6 },{ "STY", STY, ZP0, 3 },{ "STA", STA, ZP0, 3 },{ "STX", STX, ZP0, 3 },{ "???", XXX, IMP, 3 },{ "DEY", DEY, IMP, 2 },{ "???", NOP, IMP, 2 },{ "TXA", TXA, IMP, 2 },{ "???", XXX, IMP, 2 },{ "STY", STY, ABS, 4 },{ "STA", STA, ABS, 4 },{ "STX", STX, ABS, 4 },{ "???", XXX, IMP, 4 },
        { "BCC", BCC, REL, 2 },{ "STA", STA, IZY, 6 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 6 },{ "STY", STY, ZPX, 4 },{ "STA", STA, ZPX, 4 },{ "STX", STX, ZPY, 4 },{ "???", XXX, IMP, 4 },{ "TYA", TYA, IMP, 2 },{ "STA", STA, ABY, 5 },{ "TXS", TXS, IMP, 2 },{ "???", XXX, IMP, 5 },{ "???", NOP, IMP, 5 },{ "STA", STA, ABX, 5 },{ "???", XXX, IMP, 5 },{ "???", XXX, IMP, 5 },
        { "LDY", LDY, IMM, 2 },{ "LDA", LDA, IZX, 6 },{ "LDX", LDX, IMM, 2 },{ "???", XXX, IMP, 6 },{ "LDY", LDY, ZP0, 3 },{ "LDA", LDA, ZP0, 3 },{ "LDX", LDX, ZP0, 3 },{ "???", XXX, IMP, 3 },{ "TAY", TAY, IMP, 2 },{ "LDA", LDA, IMM, 2 },{ "TAX", TAX, IMP, 2 },{ "???", XXX, IMP, 2 },{ "LDY", LDY, ABS, 4 },{ "LDA", LDA, ABS, 4 },{ "LDX", LDX, ABS, 4 },{ "???", XXX, IMP, 4 },
        { "BCS", BCS, REL, 2 },{ "LDA", LDA, IZY, 5 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 5 },{ "LDY", LDY, ZPX, 4 },{ "LDA", LDA, ZPX, 4 },{ "LDX", LDX, ZPY, 4 },{ "???", XXX, IMP, 4 },{ "CLV", CLV, IMP, 2 },{ "LDA", LDA, ABY, 4 },{ "TSX", TSX, IMP, 2 },{ "???", XXX, IMP, 4 },{ "LDY", LDY, ABX, 4 },{ "LDA", LDA, ABX, 4 },{ "LDX", LDX, ABY, 4 },{ "???", XXX, IMP, 4 },
        { "CPY", CPY, IMM, 2 },{ "CMP", CMP, IZX, 6 },{ "???", NOP, IMP, 2 },{ "???", XXX, IMP, 8 },{ "CPY", CPY, ZP0, 3 },{ "CMP", CMP, ZP0, 3 },{ "DEC", DEC, ZP0, 5 },{ "???", XXX, IMP, 5 },{ "INY", INY, IMP, 2 },{ "CMP", CMP, IMM, 2 },{ "DEX", DEX, IMP, 2 },{ "???", XXX, IMP, 2 },{ "CPY", CPY, ABS, 4 },{ "CMP", CMP, ABS, 4 },{ "DEC", DEC, ABS, 6 },{ "???", XXX, IMP, 6 },
        { "BNE", BNE, REL, 2 },{ "CMP", CMP, IZY, 5 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 8 },{ "???", NOP, IMP, 4 },{ "CMP", CMP, ZPX, 4 },{ "DEC", DEC, ZPX, 6 },{ "???", XXX, IMP, 6 },{ "CLD", CLD, IMP, 2 },{ "CMP", CMP, ABY, 4 },{ "NOP", NOP, IMP, 2 },{ "???", XXX, IMP, 7 },{ "???", NOP, IMP, 4 },{ "CMP", CMP, ABX, 4 },{ "DEC", DEC, ABX, 7 },{ "???", XXX, IMP, 7 },
        { "CPX", CPX, IMM, 2 },{ "SBC", SBC, IZX, 6 },{ "???", NOP, IMP, 2 },{ "???", XXX, IMP, 8 },{ "CPX", CPX, ZP0, 3 },{ "SBC", SBC, ZP0, 3 },{ "INC", INC, ZP0, 5 },{ "???", XXX, IMP, 5 },{ "INX", INX, IMP, 2 },{ "SBC", SBC, IMM, 2 },{ "NOP", NOP, IMP, 2 },{ "???", SBC, IMP, 2 },{ "CPX", CPX, ABS, 4 },{ "SBC", SBC, ABS, 4 },{ "INC", INC, ABS, 6 },{ "???", XXX, IMP, 6 },
        { "BEQ", BEQ, REL, 2 },{ "SBC", SBC, IZY, 5 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 8 },{ "???", NOP, IMP, 4 },{ "SBC", SBC, ZPX, 4 },{ "INC", INC, ZPX, 6 },{ "???", XXX, IMP, 6 },{ "SED", SED, IMP, 2 },{ "SBC", SBC, ABY, 4 },{ "NOP", NOP, IMP, 2 },{ "???", XXX, IMP, 7 },{ "???", NOP, IMP, 4 },{ "SBC", SBC, ABX, 4 },{ "INC", INC, ABX, 7 },{ "???", XXX, IMP, 7 },
};


void cInit(cpu6502* cpu) {
    cpu->A = 0x00;
    cpu->X = 0x00;
    cpu->Y = 0x00;
    cpu->SP = 0x00;
    cpu->PC = 0x0000;
    cpu->status = 0x00;

    cpu->fetched = 0x00;

    cpu->addr_abs = 0x0000;
    cpu->addr_rel = 0x00;
    cpu->opcode = 0x00;
    cpu->cycles = 0;

}

void connectBus(cpu6502* cpu, bus* b) {
    cpu->nesBus = b;
}

uint8_t cRead(cpu6502* cpu, uint16_t addr) {
    return cpu->nesBus->bRead(cpu->nesBus, addr, 0);
}

void cWrite(cpu6502* cpu, uint16_t addr, uint8_t val) {
    cpu->nesBus->bWrite(cpu->nesBus, addr, val);
}

uint8_t getFlag(cpu6502* cpu, CPUFLAGS flag) {
    return (cpu->status & flag) > 0; // if stuff doesn't work, do ? 1 : 0 here
}

void setFlag(cpu6502* cpu, CPUFLAGS flag, int val) {
    if (val) {
        cpu->status |= flag;
    } else {
        cpu->status &= ~flag;
    }
}



void cpuClock(cpu6502* cpu) {
    if (cpu->cycles == 0) {
        cpu->opcode = cRead(cpu,cpu->PC);
        cpu->PC++;
        cpu->cycles = lookupTable[cpu->opcode].cycles;

        uint8_t extraCycles1 = lookupTable[cpu->opcode].addressmode(cpu);
        uint8_t extraCycles2 = lookupTable[cpu->opcode].operation(cpu);

        cpu->cycles += (extraCycles1 & extraCycles2);
    }

    cpu->cycles--;
}

void reset(cpu6502* cpu) {
    cpu->A = 0;
    cpu->X = 0;
    cpu->Y = 0;
    cpu->SP = 0xFD; //
    cpu->status = 0x00 | U_FLAG;

    cpu->addr_abs = 0xFFFC;
    uint16_t lowByte = cRead(cpu, cpu->addr_abs);
    uint16_t highByte = cRead(cpu, cpu->addr_abs + 1);

    cpu->PC = (highByte << 8) | lowByte;

    cpu->addr_rel = 0x0000;
    cpu->addr_abs = 0x0000;
    cpu->fetched = 0x00;

    cpu->cycles = 8;
}

void irq(cpu6502* cpu) {
    if (getFlag(cpu, I_FLAG) == 0) {
        cWrite(cpu, 0x0100 + cpu->SP, (cpu->PC >> 8) & 0x00FF);
        cpu->SP--;
        cWrite(cpu, 0x0100 + cpu->SP, cpu->PC & 0x00FF);
        cpu->SP--;

        setFlag(cpu, B_FLAG, 0);
        setFlag(cpu, U_FLAG, 1);
        setFlag(cpu, I_FLAG, 1);
        cWrite(cpu, 0x0100 + cpu->SP, cpu->status);
        cpu->SP--;

        cpu->addr_abs = 0xFFFE;
        uint16_t lowByte = cRead(cpu, cpu->addr_abs);
        uint16_t highByte = cRead(cpu, cpu->addr_abs + 1);

        cpu->PC = (highByte << 8) | lowByte;

        cpu->cycles = 7;
    }
}

void nmi(cpu6502* cpu) {
    cWrite(cpu, 0x0100 + cpu->SP, (cpu->PC >> 8) & 0x00FF);
    cpu->SP--;
    cWrite(cpu,0x0100 + cpu->SP, cpu->PC & 0x00FF);
    cpu->SP--;

    setFlag(cpu, B_FLAG, 0);
    setFlag(cpu, U_FLAG, 1);
    setFlag(cpu, I_FLAG, 1);
    cWrite(cpu, 0x0100 + cpu->SP, cpu->status);
    cpu->SP--;

    cpu->addr_abs = 0xFFFA;
    uint16_t lowByte = cRead(cpu, cpu->addr_abs);
    uint16_t highByte = cRead(cpu, cpu->addr_abs + 1);

    cpu->PC = (highByte << 8) | lowByte;

    cpu->cycles = 8;
}

// Addressing Modes

uint8_t IMP(cpu6502* cpu) {
    cpu->fetched = cpu->A;
    return 0;
}

uint8_t IMM(cpu6502* cpu) {
    cpu->addr_abs = cpu->PC++;
    return 0;
}

uint8_t ZP0(cpu6502* cpu) {
    cpu->addr_abs = cRead(cpu, cpu->PC);
    cpu->PC++;
    cpu->addr_abs &= 0x00FF;
    return 0;
}

uint8_t ZPX(cpu6502* cpu) {
    cpu->addr_abs = (cRead(cpu, cpu->PC) + cpu->X);
    cpu->PC++;
    cpu->addr_abs &= 0x00FF;
    return 0;
}

uint8_t ZPY(cpu6502* cpu) {
    cpu->addr_abs = (cRead(cpu, cpu->PC) + cpu->Y);
    cpu->PC++;
    cpu->addr_abs &= 0x00FF;
    return 0;
}

uint8_t ABS(cpu6502* cpu) {
    uint16_t lowByte = cRead(cpu, cpu->PC);
    cpu->PC++;
    uint16_t highByte = cRead(cpu, cpu->PC);
    cpu->PC++;

    cpu->addr_abs = (highByte << 8) | lowByte;
    return 0;
}

uint8_t ABX(cpu6502* cpu) {
    uint16_t lowByte = cRead(cpu, cpu->PC);
    cpu->PC++;
    uint16_t highByte = cRead(cpu, cpu->PC);
    cpu->PC++;

    cpu->addr_abs = (highByte << 8) | lowByte;
    cpu->addr_abs += cpu->X;

    if ((cpu->addr_abs & 0xFF00) != (highByte << 8)) {
        return 1;
    } else {
        return 0;
    }
}

uint8_t ABY(cpu6502* cpu) {
    uint16_t lowByte = cRead(cpu, cpu->PC);
    cpu->PC++;
    uint16_t highByte = cRead(cpu, cpu->PC);
    cpu->PC++;

    cpu->addr_abs = (highByte << 8) | lowByte;
    cpu->addr_abs += cpu->Y;

    if ((cpu->addr_abs & 0xFF00) != (highByte << 8)) {
        return 1;
    } else {
        return 0;
    }
}

uint8_t IND(cpu6502* cpu) {
    uint16_t ptr_lowByte = cRead(cpu, cpu->PC);
    cpu->PC++;
    uint16_t ptr_highByte = cRead(cpu, cpu->PC);
    cpu->PC++;

    uint16_t ptr = (ptr_highByte << 8) | ptr_lowByte;

    // page boundary hardware bug implementation
    if (ptr_lowByte == 0x00FF) {
        cpu->addr_abs = (cRead(cpu, ptr & 0xFF00) << 8) | cRead(cpu, ptr);
    } else {
        cpu->addr_abs = (cRead(cpu, ptr + 1) << 8) | cRead(cpu, ptr);
    }

    return 0;
}

// 8-bit address offset by X register used read from page 0x00, reading the actual 16-bit address from that address

uint8_t IZX(cpu6502* cpu) {
    uint16_t base_addr = cRead(cpu, cpu->PC);
    cpu->PC++;

    uint16_t lowByte = cRead(cpu, (uint16_t)(base_addr + (uint16_t)cpu->X) & 0x00FF);
    uint16_t highByte = cRead(cpu, (uint16_t)(base_addr + (uint16_t)cpu->X + 1) & 0x00FF);

    cpu->addr_abs = (highByte << 8) | lowByte;

    return 0;
}

uint8_t IZY(cpu6502* cpu) {
    uint16_t base_addr = cRead(cpu, cpu->PC);
    cpu->PC++;

    uint16_t lowByte = cRead(cpu, base_addr & 0x00FF);
    uint16_t highByte = cRead(cpu, (base_addr + 1) & 0x00FF);

    cpu->addr_abs = (highByte << 8) | lowByte;
    cpu->addr_abs += cpu->Y;

    if ((cpu->addr_abs & 0xFF00) != (highByte << 8)) {
        return 1;
    } else {
        return 0;
    }
}

uint8_t REL(cpu6502* cpu) {
    cpu->addr_rel = cRead(cpu, cpu->PC);
    cpu->PC++;
    if (cpu->addr_rel & 0x80) {
        cpu->addr_rel |= 0xFF00;
    }
    return 0;
}

// Instructions

uint8_t fetchData(cpu6502* cpu) {
    if (lookupTable[cpu->opcode].addressmode != IMP) {
        cpu->fetched = cRead(cpu, cpu->addr_abs);
    }
    return cpu->fetched;
}

// Arithmetic

// ----------------------------------

// Add with carry, flags are set based on the result
// Overflow flag is set with formula > V = ~(A ^ M) & (A ^ R)

uint8_t ADC(cpu6502* cpu) {
    fetchData(cpu);
    uint16_t result = (uint16_t)cpu->A + (uint16_t)cpu->fetched + (uint16_t)getFlag(cpu, C_FLAG);

    setFlag(cpu, C_FLAG, result > 255);
    setFlag(cpu, Z_FLAG, (result & 0x00FF) == 0);
    setFlag(cpu, N_FLAG, result & 0x80);
    setFlag(cpu, V_FLAG, (~(uint16_t)cpu->A ^ (uint16_t)cpu->fetched) & ((uint16_t)cpu->A ^ result) & 0x0080);

    cpu->A = result & 0x00FF;
    return 1;
}

// 2 complement is used on the value to be subtracted and the carry flag is used to indicate a borrow
// otherwise, very similar to ADC

uint8_t SBC(cpu6502* cpu) {
    fetchData(cpu);
    uint16_t inverted = ((uint16_t)cpu->fetched) ^ 0x00FF;
    uint16_t result = (uint16_t)cpu->A + inverted + (uint16_t)getFlag(cpu, C_FLAG);

    setFlag(cpu, C_FLAG, result & 0xFF00);
    setFlag(cpu, Z_FLAG, (result & 0x00FF) == 0);
    setFlag(cpu, N_FLAG, result & 0x0080);
    setFlag(cpu, V_FLAG, (result ^ (uint16_t)cpu->A) & (result ^ inverted) & 0x0080);

    cpu->A = result & 0x00FF;
    return 1;
}

// Bitwise AND

uint8_t AND(cpu6502* cpu) {
    fetchData(cpu);
    cpu->A = cpu->A & cpu->fetched;
    setFlag(cpu, Z_FLAG, cpu->A == 0x00);
    setFlag(cpu, N_FLAG, cpu->A & 0x80);
    return 1;
}

// Bitwise OR

uint8_t ORA(cpu6502* cpu) {
    fetchData(cpu);
    cpu->A = cpu->A | cpu->fetched;
    setFlag(cpu, Z_FLAG, cpu->A == 0x00);
    setFlag(cpu, N_FLAG, cpu->A & 0x80);
    return 1;
}

// Bitwise XOR

uint8_t EOR(cpu6502* cpu) {
    fetchData(cpu);
    cpu->A = cpu->A ^ cpu->fetched;
    setFlag(cpu, Z_FLAG, cpu->A == 0x00);
    setFlag(cpu, N_FLAG, cpu->A & 0x80);
    return 1;
}

// Shift left

uint8_t ASL(cpu6502* cpu) {
    fetchData(cpu);
    uint16_t result = (uint16_t)cpu->fetched << 1;
    setFlag(cpu, C_FLAG, result & 0xFF00);
    setFlag(cpu, Z_FLAG, (result & 0x00FF) == 0);
    setFlag(cpu, N_FLAG, result & 0x80);

    if (lookupTable[cpu->opcode].addressmode == IMP) {
        cpu->A = result & 0x00FF;
    } else {
        cWrite(cpu, cpu->addr_abs, result & 0x00FF);
    }

    return 0;
}

// Shift right

uint8_t LSR(cpu6502* cpu) {
    fetchData(cpu);
    setFlag(cpu, C_FLAG, cpu->fetched & 0x0001);
    uint16_t result = cpu->fetched >> 1;
    setFlag(cpu, Z_FLAG, (result & 0x00FF) == 0);
    setFlag(cpu, N_FLAG, result & 0x0080);

    if (lookupTable[cpu->opcode].addressmode == IMP) {
        cpu->A = result & 0x00FF;
    } else {
        cWrite(cpu, cpu->addr_abs, result & 0x00FF);
    }

    return 0;
}

// Rotate left

uint8_t ROL(cpu6502* cpu) {
    fetchData(cpu);
    uint16_t result = (uint16_t)cpu->fetched << 1 | getFlag(cpu, C_FLAG);
    setFlag(cpu, C_FLAG, result & 0xFF00);
    setFlag(cpu, Z_FLAG, (result & 0x00FF) == 0);
    setFlag(cpu, N_FLAG, result & 0x0080);

    if (lookupTable[cpu->opcode].addressmode == IMP) {
        cpu->A = result & 0x00FF;
    } else {
        cWrite(cpu, cpu->addr_abs, result & 0x00FF);
    }

    return 0;
}

// Rotate right

uint8_t ROR(cpu6502* cpu) {
    fetchData(cpu);
    uint16_t result = (uint16_t)cpu->fetched >> 1 | (getFlag(cpu, C_FLAG) << 7);
    setFlag(cpu, C_FLAG, cpu->fetched & 0x01);
    setFlag(cpu, Z_FLAG, (result & 0x00FF) == 0);
    setFlag(cpu, N_FLAG, result & 0x0080);

    if (lookupTable[cpu->opcode].addressmode == IMP) {
        cpu->A = result & 0x00FF;
    } else {
        cWrite(cpu, cpu->addr_abs, result & 0x00FF);
    }

    return 0;
}

// Increment value

uint8_t INC(cpu6502* cpu) {
    fetchData(cpu);
    uint16_t result = cpu->fetched + 1;
    cWrite(cpu, cpu->addr_abs, result & 0x00FF);
    setFlag(cpu, Z_FLAG, (result & 0x00FF) == 0);
    setFlag(cpu, N_FLAG, result & 0x0080);
    return 0;
}

// Increment X register

uint8_t INX(cpu6502* cpu) {
    cpu->X++;
    setFlag(cpu, Z_FLAG, cpu->X == 0x00);
    setFlag(cpu, N_FLAG, cpu->X & 0x80);
    return 0;
}

// Increment Y register

uint8_t INY(cpu6502* cpu) {
    cpu->Y++;
    setFlag(cpu, Z_FLAG, cpu->Y == 0x00);
    setFlag(cpu, N_FLAG, cpu->Y & 0x80);
    return 0;
}

// Decrement value

uint8_t DEC(cpu6502* cpu) {
    fetchData(cpu);
    uint16_t result = cpu->fetched - 1;
    cWrite(cpu, cpu->addr_abs, result & 0x00FF);
    setFlag(cpu, Z_FLAG, (result & 0x00FF) == 0);
    setFlag(cpu, N_FLAG, result & 0x0080);
    return 0;
}

// Decrement X register

uint8_t DEX(cpu6502* cpu) {
    cpu->X--;
    setFlag(cpu, Z_FLAG, cpu->X == 0x00);
    setFlag(cpu, N_FLAG, cpu->X & 0x80);
    return 0;
}

// Decrement Y register

uint8_t DEY(cpu6502* cpu) {
    cpu->Y--;
    setFlag(cpu, Z_FLAG, cpu->Y == 0x00);
    setFlag(cpu, N_FLAG, cpu->Y & 0x80);
    return 0;
}

// Compare

uint8_t CMP(cpu6502* cpu) {
    fetchData(cpu);
    uint16_t result = (uint16_t)cpu->A - (uint16_t)cpu->fetched;
    setFlag(cpu, C_FLAG, cpu->A >= cpu->fetched);
    setFlag(cpu, Z_FLAG, (result & 0x00FF) == 0);
    setFlag(cpu, N_FLAG, result & 0x0080);
    return 1;
}

// Compare X register

uint8_t CPX(cpu6502* cpu) {
    fetchData(cpu);
    uint16_t result = (uint16_t)cpu->X - (uint16_t)cpu->fetched;
    setFlag(cpu, C_FLAG, cpu->X >= cpu->fetched);
    setFlag(cpu, Z_FLAG, (result & 0x00FF) == 0);
    setFlag(cpu, N_FLAG, result & 0x0080);
    return 0;
}

// Compare Y register

uint8_t CPY(cpu6502* cpu) {
    fetchData(cpu);
    uint16_t result = (uint16_t)cpu->Y - (uint16_t)cpu->fetched;
    setFlag(cpu, C_FLAG, cpu->Y >= cpu->fetched);
    setFlag(cpu, Z_FLAG, (result & 0x00FF) == 0);
    setFlag(cpu, N_FLAG, result & 0x0080);
    return 0;
}



// Bit Instructions

// ----------------------------------

// Test bits in memory with accumulator
uint8_t BIT(cpu6502* cpu) {
    fetchData(cpu);
    uint16_t result = cpu->A & cpu->fetched;
    setFlag(cpu, Z_FLAG, (result & 0x00FF) == 0);
    setFlag(cpu, N_FLAG, cpu->fetched & (1 << 7));
    setFlag(cpu, V_FLAG, cpu->fetched & (1 << 6));
    return 0;
}

// Clear the carry bit
uint8_t CLC(cpu6502* cpu) {
    setFlag(cpu, C_FLAG, 0);
    return 0;
}

// Set the carry bit
uint8_t SEC(cpu6502* cpu) {
    setFlag(cpu, C_FLAG, 1);
    return 0;
}

// Clear the interrupt disable bit
uint8_t CLI(cpu6502* cpu) {
    setFlag(cpu, I_FLAG, 0);
    return 0;
}

// Set the interrupt disable bit
uint8_t SEI(cpu6502* cpu) {
    setFlag(cpu, I_FLAG, 1);
    return 0;
}

// Clear the overflow bit
uint8_t CLV(cpu6502* cpu) {
    setFlag(cpu, V_FLAG, 0);
    return 0;
}

// Clear the decimal mode bit
uint8_t CLD(cpu6502* cpu) {
    setFlag(cpu, D_FLAG, 0);
    return 0;
}

// Set the decimal mode bit
uint8_t SED(cpu6502* cpu) {
    setFlag(cpu, D_FLAG, 1);
    return 0;
}

// Branch Instructions based on flags
// ----------------------------------

// Branch if carry clear
uint8_t BCC(cpu6502* cpu) {
    if (getFlag(cpu, C_FLAG) == 0) {
        cpu->cycles++;
        cpu->addr_abs = cpu->PC + cpu->addr_rel;

        if ((cpu->addr_abs & 0xFF00) != (cpu->PC & 0xFF00)) {
            cpu->cycles++;
        }

        cpu->PC = cpu->addr_abs;
    }
    return 0;
}

// Branch if carry set
uint8_t BCS(cpu6502* cpu) {
    if (getFlag(cpu, C_FLAG) == 1) {
        cpu->cycles++;
        cpu->addr_abs = cpu->PC + cpu->addr_rel;

        if ((cpu->addr_abs & 0xFF00) != (cpu->PC & 0xFF00)) {
            cpu->cycles++;
        }

        cpu->PC = cpu->addr_abs;
    }
    return 0;
}

// Branch if equal
uint8_t BEQ(cpu6502* cpu) {
    if (getFlag(cpu, Z_FLAG) == 1) {
        cpu->cycles++;
        cpu->addr_abs = cpu->PC + cpu->addr_rel;

        if ((cpu->addr_abs & 0xFF00) != (cpu->PC & 0xFF00)) {
            cpu->cycles++;
        }

        cpu->PC = cpu->addr_abs;
    }
    return 0;
}

// Branch if not equal
uint8_t BNE(cpu6502* cpu) {
    if (getFlag(cpu, Z_FLAG) == 0) {
        cpu->cycles++;
        cpu->addr_abs = cpu->PC + cpu->addr_rel;

        if ((cpu->addr_abs & 0xFF00) != (cpu->PC & 0xFF00)) {
            cpu->cycles++;
        }

        cpu->PC = cpu->addr_abs;
    }
    return 0;
}

// Branch if negative
uint8_t BMI(cpu6502* cpu) {
    if (getFlag(cpu, N_FLAG) == 1) {
        cpu->cycles++;
        cpu->addr_abs = cpu->PC + cpu->addr_rel;

        if ((cpu->addr_abs & 0xFF00) != (cpu->PC & 0xFF00)) {
            cpu->cycles++;
        }

        cpu->PC = cpu->addr_abs;
    }
    return 0;
}

// Branch if positive
uint8_t BPL(cpu6502* cpu) {
    if (getFlag(cpu, N_FLAG) == 0) {
        cpu->cycles++;
        cpu->addr_abs = cpu->PC + cpu->addr_rel;

        if ((cpu->addr_abs & 0xFF00) != (cpu->PC & 0xFF00)) {
            cpu->cycles++;
        }

        cpu->PC = cpu->addr_abs;
    }
    return 0;
}

// Branch if overflow clear
uint8_t BVC(cpu6502* cpu) {
    if (getFlag(cpu, V_FLAG) == 0) {
        cpu->cycles++;
        cpu->addr_abs = cpu->PC + cpu->addr_rel;

        if ((cpu->addr_abs & 0xFF00) != (cpu->PC & 0xFF00)) {
            cpu->cycles++;
        }

        cpu->PC = cpu->addr_abs;
    }
    return 0;
}

// Branch if overflow set
uint8_t BVS(cpu6502* cpu) {
    if (getFlag(cpu, V_FLAG) == 1) {
        cpu->cycles++;
        cpu->addr_abs = cpu->PC + cpu->addr_rel;

        if ((cpu->addr_abs & 0xFF00) != (cpu->PC & 0xFF00)) {
            cpu->cycles++;
        }

        cpu->PC = cpu->addr_abs;
    }
    return 0;
}


// Stack Instructions

// ----------------------------------

// Push the accumulator to the stack
uint8_t PHA(cpu6502* cpu) {
    cWrite(cpu, 0x0100 + cpu->SP, cpu->A); // Stack is located at 0x0100
    cpu->SP--;
    return 0;
}

// Pop the accumulator from the stack
uint8_t PLA(cpu6502* cpu) {
    cpu->SP++;
    cpu->A = cRead(cpu, 0x0100 + cpu->SP);
    setFlag(cpu, Z_FLAG, cpu->A == 0x00);
    setFlag(cpu, N_FLAG, cpu->A & 0x80);
    return 0;
}

// Push the status register to the stack
uint8_t PHP(cpu6502* cpu) {
    cWrite(cpu, 0x0100 + cpu->SP, cpu->status);
    cpu->SP--;
    return 0;
}

// Pop the status register from the stack
uint8_t PLP(cpu6502* cpu) {
    cpu->SP++;
    cpu->status = cRead(cpu, 0x0100 + cpu->SP);
    setFlag(cpu, U_FLAG, 1);
    return 0;
}

// Return from interrupt
uint8_t RTI(cpu6502* cpu) {
    cpu->SP++;
    cpu->status = cRead(cpu, 0x0100 + cpu->SP);
    cpu->status &= ~B_FLAG;
    cpu->status &= ~U_FLAG;

    cpu->SP++;
    cpu->PC = (uint16_t)cRead(cpu, 0x0100 + cpu->SP);
    cpu->SP++;
    cpu->PC |= (uint16_t)cRead(cpu, 0x0100 + cpu->SP) << 8;

    return 0;
}

// Return from subroutine
uint8_t RTS(cpu6502* cpu) {
    cpu->SP++;
    cpu->PC = (uint16_t)cRead(cpu, 0x0100 + cpu->SP);
    cpu->SP++;
    cpu->PC |= (uint16_t)cRead(cpu, 0x0100 + cpu->SP) << 8;

    cpu->PC++;
    return 0;
}

// Jump to subroutine
uint8_t JSR(cpu6502* cpu) {
    cpu->PC--;

    cWrite(cpu, 0x0100 + cpu->SP, (cpu->PC >> 8) & 0x00FF);
    cpu->SP--;
    cWrite(cpu, 0x0100 + cpu->SP, cpu->PC & 0x00FF);
    cpu->SP--;

    cpu->PC = cpu->addr_abs;
    return 0;
}

// Jump
uint8_t JMP(cpu6502* cpu) {
    cpu->PC = cpu->addr_abs;
    return 0;
}

// Break
uint8_t BRK(cpu6502* cpu) {
    cpu->PC++;

    setFlag(cpu, I_FLAG, 1);
    cWrite(cpu, 0x0100 + cpu->SP, (cpu->PC >> 8) & 0x00FF);
    cpu->SP--;
    cWrite(cpu, 0x0100 + cpu->SP, cpu->PC & 0x00FF);
    cpu->SP--;

    setFlag(cpu, B_FLAG, 1);
    cWrite(cpu, 0x0100 + cpu->SP, cpu->status);
    cpu->SP--;
    setFlag(cpu, B_FLAG, 0);

    cpu->PC = (uint16_t)cRead(cpu, 0xFFFE) | ((uint16_t)cRead(cpu, 0xFFFF) << 8);

    return 0;
}


// System Instructions

// ----------------------------------

// No operation

uint8_t NOP(cpu6502* cpu) {

    // NOPs for unofficial opcodes could be added here
    if(complete(cpu)){
        return 0;
    }
    return 0;
}

// Transfer Instructions

// ----------------------------------

// Load the accumulator with a value
uint8_t LDA(cpu6502* cpu) {
    fetchData(cpu);
    cpu->A = cpu->fetched;
    setFlag(cpu, Z_FLAG, cpu->A == 0x00);
    setFlag(cpu, N_FLAG, cpu->A & 0x80);
    return 1;
}

// Load the X register with a value
uint8_t LDX(cpu6502* cpu) {
    fetchData(cpu);
    cpu->X = cpu->fetched;
    setFlag(cpu, Z_FLAG, cpu->X == 0x00);
    setFlag(cpu, N_FLAG, cpu->X & 0x80);
    return 1;
}

// Load the Y register with a value
uint8_t LDY(cpu6502* cpu) {
    fetchData(cpu);
    cpu->Y = cpu->fetched;
    setFlag(cpu, Z_FLAG, cpu->Y == 0x00);
    setFlag(cpu, N_FLAG, cpu->Y & 0x80);
    return 1;
}

// Transfer the accumulator to the X register
uint8_t TAX(cpu6502* cpu) {
    cpu->X = cpu->A;
    setFlag(cpu, Z_FLAG, cpu->X == 0x00);
    setFlag(cpu, N_FLAG, cpu->X & 0x80);
    return 0;
}

// Transfer the accumulator to the Y register
uint8_t TAY(cpu6502* cpu) {
    cpu->Y = cpu->A;
    setFlag(cpu, Z_FLAG, cpu->Y == 0x00);
    setFlag(cpu, N_FLAG, cpu->Y & 0x80);
    return 0;
}

// Transfer the X register to the accumulator
uint8_t TXA(cpu6502* cpu) {
    cpu->A = cpu->X;
    setFlag(cpu, Z_FLAG, cpu->A == 0x00);
    setFlag(cpu, N_FLAG, cpu->A & 0x80);
    return 0;
}

// Transfer the Y register to the accumulator
uint8_t TYA(cpu6502* cpu) {
    cpu->A = cpu->Y;
    setFlag(cpu, Z_FLAG, cpu->A == 0x00);
    setFlag(cpu, N_FLAG, cpu->A & 0x80);
    return 0;
}

// Transfer the stack pointer to the X register
uint8_t TSX(cpu6502* cpu) {
    cpu->X = cpu->SP;
    setFlag(cpu, Z_FLAG, cpu->X == 0x00);
    setFlag(cpu, N_FLAG, cpu->X & 0x80);
    return 0;
}

// Transfer the X register to the stack pointer
uint8_t TXS(cpu6502* cpu) {
    cpu->SP = cpu->X;
    return 0;
}

// Store the accumulator in memory
uint8_t STA(cpu6502* cpu) {
    cWrite(cpu, cpu->addr_abs, cpu->A);
    return 0;
}

// Store the X register in memory
uint8_t STX(cpu6502* cpu) {
    cWrite(cpu, cpu->addr_abs, cpu->X);
    return 0;
}

// Store the Y register in memory
uint8_t STY(cpu6502* cpu) {
    cWrite(cpu, cpu->addr_abs, cpu->Y);
    return 0;
}

// Unofficial Instructions

// ----------------------------------

uint8_t XXX(cpu6502* cpu) {
    if(complete(cpu)){
        return 0;
    }
    return 0;
}




// Utility

// ----------------------------------


Instruction_Map createMap() {
    Instruction_Map map;
    map.count = 0;
    map.capacity = MAP_LENGTH;
    // fill
    for (int i = 0; i < MAP_LENGTH; i++) {
        map.entries[i].address = 0;
        strcpy(map.entries[i].instruction, "\0");
        //entry.instruction = "\0";
    }
    return map;
}

void destroyMap() {
}

void addInstruction(Instruction_Map *map, uint16_t address,char *instruction) {
    map->entries[address].address = address;
    strncpy(map->entries[address].instruction, instruction, MAX_INSTRUCTION_LENGTH);
    map->entries[address].instruction[MAX_INSTRUCTION_LENGTH - 1] = '\0';
}

void hex(uint32_t intValue, uint8_t length, char* hexstring){
    for(int i = length - 1; i >= 0; i--, intValue >>= 4){
        hexstring[i] = "0123456789ABCDEF"[intValue & 0xF];
    }
    hexstring[length] = '\0';
}

int complete(cpu6502* cpu) {
    return cpu->cycles == 0;
}

Instruction_Map disassemble(uint16_t nStart, uint16_t nStop, bus *bus) {
    uint32_t addr = nStart;
    uint8_t value = 0x00, lowByte = 0x00, highByte = 0x00;
    Instruction_Map map = createMap();
    char sInst[MAX_INSTRUCTION_LENGTH]; // Instruction string

    while (addr <= (uint32_t)nStop) {
        uint16_t line_addr = addr;
        char addr_buf[6]; // Buffer for formatted address

        // Prefix line with instruction address
        hex(addr, 4, addr_buf);
        snprintf(sInst, sizeof(sInst), "$%s: ", addr_buf);

        // Read instruction, and get its readable name
        uint8_t opcode = bus->bRead(bus, addr, 1); addr++;
        strncat(sInst, lookupTable[opcode].name, sizeof(sInst) - strlen(sInst) - 1); // Append instruction name
        strncat(sInst, " ", sizeof(sInst) - strlen(sInst) - 1);

        // Get operands based on addressing mode in C


        if (lookupTable[opcode].addressmode == &IMP) {
            strncat(sInst, "{IMP}", sizeof(sInst) - strlen(sInst) - 1);
        }
        else if (lookupTable[opcode].addressmode == &IMM)
        {
            value = bus->bRead(bus, addr, 1); addr++;
            char value_buf[3];
            hex(value, 2, value_buf);
            snprintf(sInst + strlen(sInst), sizeof(sInst) - strlen(sInst), "#$%s {IMM}", value_buf);
        }
        else if (lookupTable[opcode].addressmode == &ZP0)
        {
            value = bus->bRead(bus, addr, 1); addr++;
            char value_buf[3];
            hex(value, 2, value_buf);
            snprintf(sInst + strlen(sInst), sizeof(sInst) - strlen(sInst), "$%s {ZP0}", value_buf);
        }
        else if (lookupTable[opcode].addressmode == &ZPX)
        {
            value = bus->bRead(bus, addr, 1); addr++;
            char value_buf[3];
            hex(value, 2, value_buf);
            snprintf(sInst + strlen(sInst), sizeof(sInst) - strlen(sInst), "$%s, X {ZPX}", value_buf);
        }
        else if (lookupTable[opcode].addressmode == &ZPY)
        {
            value = bus->bRead(bus, addr, 1); addr++;
            char value_buf[3];
            hex(value, 2, value_buf);
            snprintf(sInst + strlen(sInst), sizeof(sInst) - strlen(sInst), "$%s, Y {ZPY}", value_buf);
        }
        else if (lookupTable[opcode].addressmode == &IZX)
        {
            value = bus->bRead(bus, addr, 1); addr++;
            char value_buf[3];
            hex(value, 2, value_buf);
            snprintf(sInst + strlen(sInst), sizeof(sInst) - strlen(sInst), "($%s, X) {IZX}", value_buf);
        }
        else if (lookupTable[opcode].addressmode == &IZY)
        {
            value = bus->bRead(bus, addr, 1); addr++;
            char value_buf[3];
            hex(value, 2, value_buf);
            snprintf(sInst + strlen(sInst), sizeof(sInst) - strlen(sInst), "($%s), Y {IZY}", value_buf);
        }
        else if (lookupTable[opcode].addressmode == &ABS)
        {
            lowByte = bus->bRead(bus, addr, 1); addr++;
            highByte = bus->bRead(bus, addr, 1); addr++;
            char value_buf[5];
            hex((highByte << 8) | lowByte, 4, value_buf);
            snprintf(sInst + strlen(sInst), sizeof(sInst) - strlen(sInst), "$%s {ABS}", value_buf);
        }
        else if (lookupTable[opcode].addressmode == &ABX)
        {
            lowByte = bus->bRead(bus, addr, 1); addr++;
            highByte = bus->bRead(bus, addr, 1); addr++;
            char value_buf[5];
            hex((highByte << 8) | lowByte, 4, value_buf);
            snprintf(sInst + strlen(sInst), sizeof(sInst) - strlen(sInst), "$%s, X {ABX}", value_buf);
        }
        else if (lookupTable[opcode].addressmode == &ABY)
        {
            lowByte = bus->bRead(bus, addr, 1); addr++;
            highByte = bus->bRead(bus, addr, 1); addr++;
            char value_buf[5];
            hex((highByte << 8) | lowByte, 4, value_buf);
            snprintf(sInst + strlen(sInst), sizeof(sInst) - strlen(sInst), "$%s, Y {ABY}", value_buf);
        }
        else if (lookupTable[opcode].addressmode == &IND)
        {
            lowByte = bus->bRead(bus, addr, 1); addr++;
            highByte = bus->bRead(bus, addr, 1); addr++;
            char value_buf[5];
            hex((highByte << 8) | lowByte, 4, value_buf);
            snprintf(sInst + strlen(sInst), sizeof(sInst) - strlen(sInst), "($%s) {IND}", value_buf);
        }
        else if (lookupTable[opcode].addressmode == &REL)
        {
            value = bus->bRead(bus, addr, 1); addr++;
            char value_buf[3];
            hex(value, 2, value_buf);
            char value_buf2[5];
            hex(addr + (int8_t)value, 4, value_buf2);
            snprintf(sInst + strlen(sInst), sizeof(sInst) - strlen(sInst), "$%s [$%s] {REL}", value_buf, value_buf2);
        }

        // Add the formed instruction string to the map
        addInstruction(&map, line_addr, sInst);

    }

    return map;
}