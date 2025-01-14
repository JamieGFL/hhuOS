#ifndef CPU6502_H
#define CPU6502_H

#include <stdint.h>
#include "bus_interface.h"

#define MAX_INSTRUCTION_LENGTH 64
#define MAP_LENGTH (64 * 1024)

// Bits of the status register
typedef enum
{
    C_FLAG = (1 << 0), // Carry Bit
    Z_FLAG = (1 << 1), // Zero
    I_FLAG = (1 << 2), // Disable Interrupts
    D_FLAG = (1 << 3), // Decimal Mode (unused in this implementation)
    B_FLAG = (1 << 4), // Break
    U_FLAG = (1 << 5), // Unused
    V_FLAG = (1 << 6), // Overflow
    N_FLAG = (1 << 7)  // Negative
} CPUFLAGS;

typedef struct cpu6502 {
    bus* nesBus;

    uint8_t A;      // Accumulator Register
    uint8_t X;      // X Register
    uint8_t Y;      // Y Register
    uint8_t SP;   // Stack Pointer (points to location on bus)
    uint16_t PC;    // Program Counter
    uint8_t status; // Status Register

    uint8_t fetched; // fetched data

    uint16_t addr_abs; // Represents the absolute address
    uint16_t addr_rel; // Represents the relative address
    uint8_t opcode; // Represents the current opcode
    uint8_t cycles; // Represents the number of cycles the current instruction requires

} cpu6502;

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the CPU
void cInit(cpu6502* cpu);

// Connect the CPU to the bus
void connectBus(cpu6502* cpu, bus* b);

// Write to memory through bus
uint8_t cRead(cpu6502* cpu, uint16_t addr);

// Read from memory through bus
void cWrite(cpu6502* cpu, uint16_t addr, uint8_t val);

void cpuClock(cpu6502* cpu); // Simulates a clock cycle of the CPU
void cpuReset(cpu6502* cpu); // Reset Signal
void irq(cpu6502* cpu); // Interrupt Request Signal
void nmi(cpu6502* cpu); // Non-Maskable Interrupt Request Signal

uint8_t fetchData(cpu6502* cpu); // Fetches data from memory

// Get flag from status register
uint8_t getFlag(cpu6502* cpu, CPUFLAGS flag);

// Set flag in status register
void setFlag(cpu6502* cpu, CPUFLAGS flag, int val);

// Addressing Modes
uint8_t IMP(cpu6502 * cpu);
uint8_t IMM(cpu6502 * cpu);
uint8_t ZP0(cpu6502 * cpu);
uint8_t ZPX(cpu6502 * cpu);
uint8_t ZPY(cpu6502 * cpu);
uint8_t REL(cpu6502 * cpu);
uint8_t ABS(cpu6502 * cpu);
uint8_t ABX(cpu6502 * cpu);
uint8_t ABY(cpu6502 * cpu);
uint8_t IND(cpu6502 * cpu);
uint8_t IZX(cpu6502 * cpu);
uint8_t IZY(cpu6502 * cpu);

// Opcodes
uint8_t ADC(cpu6502* cpu); uint8_t AND(cpu6502* cpu); uint8_t ASL(cpu6502* cpu); uint8_t BCC(cpu6502* cpu); uint8_t BCS(cpu6502* cpu);
uint8_t BEQ(cpu6502* cpu); uint8_t BIT(cpu6502* cpu); uint8_t BMI(cpu6502* cpu); uint8_t BNE(cpu6502* cpu); uint8_t BPL(cpu6502* cpu);
uint8_t BRK(cpu6502* cpu); uint8_t BVC(cpu6502* cpu); uint8_t BVS(cpu6502* cpu); uint8_t CLC(cpu6502* cpu); uint8_t CLD(cpu6502* cpu);
uint8_t CLI(cpu6502* cpu); uint8_t CLV(cpu6502* cpu); uint8_t CMP(cpu6502* cpu); uint8_t CPX(cpu6502* cpu); uint8_t CPY(cpu6502* cpu);
uint8_t DEC(cpu6502* cpu); uint8_t DEX(cpu6502* cpu); uint8_t DEY(cpu6502* cpu); uint8_t EOR(cpu6502* cpu); uint8_t INC(cpu6502* cpu);
uint8_t INX(cpu6502* cpu); uint8_t INY(cpu6502* cpu); uint8_t JMP(cpu6502* cpu); uint8_t JSR(cpu6502* cpu); uint8_t LDA(cpu6502* cpu);
uint8_t LDX(cpu6502* cpu); uint8_t LDY(cpu6502* cpu); uint8_t LSR(cpu6502* cpu); uint8_t NOP(cpu6502* cpu); uint8_t ORA(cpu6502* cpu);
uint8_t PHA(cpu6502* cpu); uint8_t PHP(cpu6502* cpu); uint8_t PLA(cpu6502* cpu); uint8_t PLP(cpu6502* cpu); uint8_t ROL(cpu6502* cpu);
uint8_t ROR(cpu6502* cpu); uint8_t RTI(cpu6502* cpu); uint8_t RTS(cpu6502* cpu); uint8_t SBC(cpu6502* cpu); uint8_t SEC(cpu6502* cpu);
uint8_t SED(cpu6502* cpu); uint8_t SEI(cpu6502* cpu); uint8_t STA(cpu6502* cpu); uint8_t STX(cpu6502* cpu); uint8_t STY(cpu6502* cpu);
uint8_t TAX(cpu6502* cpu); uint8_t TAY(cpu6502* cpu); uint8_t TSX(cpu6502* cpu); uint8_t TXA(cpu6502* cpu); uint8_t TXS(cpu6502* cpu);
uint8_t TYA(cpu6502* cpu); uint8_t XXX(cpu6502* cpu);

// Utility
typedef struct
{
    char name[4]; // Name of the instruction
    uint8_t (*operation)(cpu6502* cpu); // = nullptr; // Function pointer to the operation
    uint8_t (*addressmode)(cpu6502* cpu); // = nullptr; // Function pointer to the address mode
    uint8_t cycles; // Number of cycles the instruction requires
} instruction;

typedef struct {
    uint16_t address;
    char instruction[MAX_INSTRUCTION_LENGTH];
} mapEntry;

typedef struct {
    mapEntry entries[MAP_LENGTH];
    int count;
    int capacity;
} Instruction_Map;

// Checks if the CPu has finished executing the current instruction
int complete(cpu6502* cpu);

// Disassembles the code in the specified range
Instruction_Map disassemble(uint16_t nStart, uint16_t nStop, bus *bus);

#ifdef __cplusplus
}
#endif

#endif // CPU6502_H