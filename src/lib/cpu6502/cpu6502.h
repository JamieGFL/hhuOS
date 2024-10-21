#pragma once
#include <stdint.h>
#include <bus.h>

// bits of the status register
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

typedef struct {
    bus* bus;

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

void cInit();

void cDestroy();

void connectBus(bus* b);

void clock(); // Clock function for clock cycles to occur
void reset(); // Reset Signal
void irq(); // Interrupt Request Signal
void nmi(); // Non-Maskable Interrupt Request Signal

uint8_t fetchData(); // Fetches data from memory

// write to memory through bus
uint8_t cRead(uint16_t addr);

// read from memory through bus
void cWrite(uint16_t addr, uint8_t val);

// get flag from status register
uint8_t getFlag(CPUFLAGS flag);

// set flag in status register
void setFlag(CPUFLAGS flag, int val);

typedef struct
{
    char name[4]; // Name of the instruction
    uint8_t (:*operation)(void) // = nullptr; // Function pointer to the operation
    uint8_t (*addressmode)(void) // = nullptr; // Function pointer to the address mode
    uint8_t cycles; // Number of cycles the instruction requires
} instruction;


// Addressing Modes
uint8_t IMP();
uint8_t IMM();
uint8_t ZP0();
uint8_t ZPX();
uint8_t ZPY();
uint8_t REL();
uint8_t ABS();
uint8_t ABX();
uint8_t ABY();
uint8_t IND();
uint8_t IZX();
uint8_t IZY();

// Opcodes
uint8_t ADC(); uint8_t AND(); uint8_t ASL(); uint8_t BCC(); uint8_t BCS(); uint8_t BEQ(); uint8_t BIT(); uint8_t BMI();
uint8_t BNE(); uint8_t BPL(); uint8_t BRK(); uint8_t BVC(); uint8_t BVS(); uint8_t CLC(); uint8_t CLD(); uint8_t CLI();
uint8_t CLV(); uint8_t CMP(); uint8_t CPX(); uint8_t CPY(); uint8_t DEC(); uint8_t DEX(); uint8_t DEY(); uint8_t EOR();
uint8_t INC(); uint8_t INX(); uint8_t INY(); uint8_t JMP(); uint8_t JSR(); uint8_t LDA(); uint8_t LDX(); uint8_t LDY();
uint8_t LSR(); uint8_t NOP(); uint8_t ORA(); uint8_t PHA(); uint8_t PHP(); uint8_t PLA(); uint8_t PLP(); uint8_t ROL();
uint8_t ROR(); uint8_t RTI(); uint8_t RTS(); uint8_t SBC(); uint8_t SEC(); uint8_t SED(); uint8_t SEI(); uint8_t STA();
uint8_t STX(); uint8_t STY(); uint8_t TAX(); uint8_t TAY(); uint8_t TSX(); uint8_t TXA(); uint8_t TXS(); uint8_t TYA();
uint8_t XXX();