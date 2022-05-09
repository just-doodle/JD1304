#ifndef __JD1304_H__
#define __JD1304_H__

#define USE_ADDRMODES 

#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <string.h>


#define SP_START 0xFF
#define SP_END 0x00
#define SP_W_ADDR 0x0100
#define RESET_VECTOR 0x8000
#define CPU_CLOCK_FREQUENCY 10

#define JD1304_VERSION "1.1.0"

struct Instruction
{
    static constexpr uint8_t LDA = 0x01;    // Load Accumulator | IMM | ZP | ZPX | ABS | ABSX | ABSY | IND | INDX | INDY |
    static constexpr uint8_t LDX = 0x02;    // Load X | IMM | ZP | ZPY | ABS | ABSY |
    static constexpr uint8_t LDY = 0x03;    // Load Y | IMM | ZP | ZPX | ABS | ABSX |
    static constexpr uint8_t STA = 0x04;    // Store Accumulator | ZP | ZPX | ABS | ABSX | ABSY | IND | INDX | INDY |
    static constexpr uint8_t STX = 0x05;    // Store X | ZP | ABS |
    static constexpr uint8_t STY = 0x06;    // Store Y | ZP | ABS |
    static constexpr uint8_t TAX = 0x07;    // Transfer Accumulator to X | IMPL |
    static constexpr uint8_t TXA = 0x08;    // Transfer X to Accumulator | IMPL |
    static constexpr uint8_t TAY = 0x09;    // Transfer Accumulator to Y | IMPL |
    static constexpr uint8_t TYA = 0x0A;    // Transfer Y to Accumulator | IMPL |
    static constexpr uint8_t TSX = 0x0B;    // Transfer Stack Pointer to X | IMPL |
    static constexpr uint8_t TXS = 0x0C;    // Transfer X to Stack Pointer | IMPL |
    static constexpr uint8_t ADD = 0x0D;    // Add | IMM | ZP | ZPX | ABS | ABSX | ABSY | IND | INDX | INDY |
    static constexpr uint8_t ADC = 0x0E;    // Add with Carry | IMM | ZP | ZPX | ABS | ABSX | ABSY | IND | INDX | INDY |
    static constexpr uint8_t SUB = 0x0F;    // Subtract | IMM | ZP | ZPX | ABS | ABSX | ABSY | IND | INDX | INDY |
    static constexpr uint8_t SBC = 0x10;    // Subtract with Carry | IMM | ZP | ZPX | ABS | ABSX | ABSY | IND | INDX | INDY |
    static constexpr uint8_t AND = 0x11;    // AND | IMM | ZP | ZPX | ABS | ABSX | ABSY | IND | INDX | INDY |
    static constexpr uint8_t OR = 0x12;     // OR | IMM | ZP | ZPX | ABS | ABSX | ABSY | IND | INDX | INDY |
    static constexpr uint8_t NOR = 0x13;    // NOR | IMM | ZP | ZPX | ABS | ABSX | ABSY | IND | INDX | INDY |
    static constexpr uint8_t CMP = 0x14;    // Compare | IMM | ZP | ZPX | ABS | ABSX | ABSY | IND | INDX | INDY |
    static constexpr uint8_t CPX = 0x15;    // Compare X | IMM | ZP | ABS |
    static constexpr uint8_t CPY = 0x16;    // Compare Y | IMM | ZP | ABS |
    static constexpr uint8_t JNZ = 0x17;    // Jump if Not Zero | IMM | ABS | ABSX | ABSY | IND | INDX | INDY |
    static constexpr uint8_t JZ = 0x18;     // Jump if Zero | IMM | ABS | ABSX | ABSY | IND | INDX | INDY |
    static constexpr uint8_t JMP = 0x19;    // Jump | ABS | ABSX | ABSY | IND | INDX | INDY |
    static constexpr uint8_t JSR = 0x1A;    // Jump to Subroutine | ABS |
    static constexpr uint8_t RET = 0x1B;    // Return from Subroutine | IMPL |
    static constexpr uint8_t IRET = 0x1C;   // Interrupt Return | IMPL |
    static constexpr uint8_t PHA = 0x1D;    // Push Accumulator | IMPL |
    static constexpr uint8_t PLA = 0x1E;    // Pull Accumulator | IMPL |
    static constexpr uint8_t PHP = 0x1F;    // Push Processor Status | IMPL |
    static constexpr uint8_t PLP = 0x20;    // Pull Processor Status | IMPL |
    static constexpr uint8_t INB = 0x21;    //! Not implemented INB | IMPL |
    static constexpr uint8_t OUTB = 0x21;   //! Not implemented OUTB | IMPL |
    static constexpr uint8_t CHAM = 0x23;   // Change Address Mode
    static constexpr uint8_t EXT = 0x24;    //! Not implemented Exit
    static constexpr uint8_t INX = 0x25;    // Increment X | IMPL |
    static constexpr uint8_t INY = 0x26;    // Increment Y | IMPL |
    static constexpr uint8_t DEX = 0x27;    // Decrement X | IMPL |
    static constexpr uint8_t DEY = 0x28;    // Decrement Y | IMPL |
    int NumInstructions = 40;
};

class Memory
{
private:
    static constexpr uint32_t MAX_MEM = 1024 * 64; // Maximum memory is 64 KB
    uint8_t RAM[MAX_MEM];                          // The memory array used
public:
    uint8_t operator[](uint16_t Address) const; // An operator to read 1 byte from memory
    uint8_t &operator[](uint16_t Address);      // An operator to write 1 byte to memory

    void init();
    Memory();
};

class CPU
{
private:
    Memory mem;

    // The main CPU Registers
    uint16_t PC; // Program counter of the CPU
    uint8_t SP; // Stack pointer of the CPU
    uint8_t status = 0x00; // Status register
    uint8_t AM; // Address mode of the current instruction

    // Index CPU Registers
    uint8_t A;     // Accumalator register
    uint8_t X;     // Index X register
    uint8_t Y;     // Index Y register

    enum flags // Flags to set in the CPU registers
    {
        C = (1 << 0), // Carry Bit
        Z = (1 << 1), // Zero
        I = (1 << 2), // Disable Interrupts
        B = (1 << 4), // Borrow
        V = (1 << 6), // Overflow
        N = (1 << 7), // Negative
    };

    uint32_t Cycles = 0x00;     // Cycles counter
    uint8_t opcode = 0x00;      // To store which opcode executed

    uint16_t StackADDR();

    void PushB(uint8_t val);
    uint8_t PopB();

    void PushW(uint16_t val);
    uint16_t PopW();

    void SetFlag(flags flag, bool val);
    bool GetFlag(flags flag);

    uint8_t FetchB();
    uint8_t ReadB(uint16_t Address);

    uint16_t FetchW();
    uint16_t ReadW(uint16_t Address);

    void WriteB(uint16_t Address, uint8_t Value);
    void WriteW(uint16_t Address, uint16_t Value);

    void IRQ();
    void NMI();

    #ifdef USE_ADDRMODES

    uint8_t tmp;

    #define AM_IMM 0x00
    #define AM_ZP0 0x01
    #define AM_ZPX 0x02
    #define AM_ZPY 0x03
    #define AM_ABS 0x04
    #define AM_ABX 0x05
    #define AM_ABY 0x06
    #define AM_IND 0x07
    #define AM_INX 0x08
    #define AM_INY 0x09
    #define AM_REL 0x0A

    // Addressing modes
    uint16_t Addr_ZP0(); // Zero Page
    uint16_t Addr_ZPX(); // Zero Page X
    uint16_t Addr_ZPY(); // Zero Page Y
    uint16_t Addr_IZX(); // Zero Page X Indirect
    uint16_t Addr_IZY(); // Zero Page Y Indirect
    uint16_t Addr_ABS(); // Absolute
    uint16_t Addr_ABX(); // Absolute X
    uint16_t Addr_ABY(); // Absolute Y
    uint16_t Addr_REL(); // Relative
    uint16_t Addr_IND(); // Indirect

    #endif

public:
    CPU();
    ~CPU();
    void ResetCPU();
    void Execute(uint32_t Cycles);
    void step();
    void Dump(char* errormsg);
    void run(int freq);
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t val);
    uint16_t read16(uint16_t addr);
    void write16(uint16_t addr, uint16_t val);

    /*
    * Function to change the registers content
    * @param reg The number of register to change [ 0: A, 1: X, 2: Y ]
    * @param val The value to set to the register
    * @return void
    */
    void ChangeREG(int regnum, uint8_t val);
};

class System
{
private:
    CPU *cpu;
public:
    static System *instance;
    System();
    void Reset();
    void load(char* filename);
    void load(uint8_t* data, uint32_t size);
    void save(char* filename);
    void dump();
    void run();
    void step();

    /*
    * @brief A wrapper to the CPU::ChangeREG function
    * @param regnum The number of register to change [ 0: A, 1: X, 2: Y,]
    * @param val The value to set to the register
    */
    void change_reg(int regnum, uint8_t val);
};

#endif