#include "JD1304.h"

System* System::instance = 0;

#define KB 1024
#define MB (1024*KB)
#define GB (1024*MB)

char* byte(uint32_t num)
{
    char* ret = new char[10];
    if(((num / MB) >> 10) != 0)
    {
        sprintf(ret, "%dGB ", num / GB);
    }
    else if (((num / KB) >> 10) != 0)
    {
        sprintf(ret, "%dMB ", num / MB);
    }
    else if (((num) >> 10) != 0)
    {
        sprintf(ret, "%dKB ", num / KB);
    }
    else
    {
        sprintf(ret, "%dB ", num);
    }
    return ret;
}

Memory::Memory()
{
    
}

void Memory::init()
{
    printf("Memory found: %s\n", strndup(byte(MAX_MEM), strlen(byte(MAX_MEM)) - 1));
    printf("Initializing memory...\n");
    // Initialize the memory with 0xFF
    for (int i = 0; i < 0xFFFF; i++)
    {
        RAM[i] = 0x00;
    }
}

// An operator to read 1 byte from memory
uint8_t Memory::operator[](uint16_t Address) const
{
    return RAM[Address];
}

// An operator to write 1 byte to memory
uint8_t& Memory::operator[](uint16_t Address)
{
    return RAM[Address];
}

CPU::CPU()
{
}

void CPU::ResetCPU()
{
    PC = RESET_VECTOR;
    SP = SP_START;
    A = 0;
    X = 0;
    Y = 0;
    mem.init();
}

uint8_t CPU::FetchB()
{
    uint8_t res = mem[PC];
    PC++;
    Cycles--;
    return res;
}

uint8_t CPU::ReadB(uint16_t addr)
{
    uint8_t Result = mem[addr];
    Cycles--;
    return Result;
}

uint16_t CPU::FetchW()
{
    uint16_t res = mem[PC] | (mem[PC + 1] << 8);
    Cycles--;
    Cycles--;
    PC += 2;
    return res;
}

uint16_t CPU::ReadW(uint16_t addr)
{
    uint16_t res = mem[addr] | (mem[addr + 1] << 8);
    Cycles--;
    Cycles--;
    return res;
}

void CPU::WriteB(uint16_t addr, uint8_t val)
{
    mem[addr] = val;
    Cycles--;
}

// 0x[ba]
void CPU::WriteW(uint16_t addr, uint16_t val)
{
    mem[addr] = val & 0xFF;
    mem[addr + 1] = (val >> 8) & 0xFF;
    Cycles--;
    Cycles--;
}

/*
* This Processor uses Descending stack [LIFO: Last In First Out]
*        -
* |__|<- SP 
* |##|   +  
* |##|      
* |##|      
* |##|      🠗 Increasing Address
* |__|
* 
* Push = SP--
* Pop = SP++
*/
uint16_t CPU::StackADDR()
{
    return SP_W_ADDR | SP;
}

void CPU::PushB(uint8_t val)
{
    const uint16_t addr = StackADDR();
    Cycles--;
    mem[addr] = val;
    SP--;
    Cycles--;
}

void CPU::PushW(uint16_t val)
{
    WriteB(StackADDR(), val >> 8);
    SP--;
    WriteB(StackADDR(), val & 0xFF);
    SP--;
    printf("Pushing %04X to stack addr %04x\n", val, StackADDR() + 2);
}

uint8_t CPU::PopB()
{
    SP++;
    Cycles--;
    const uint16_t addr = StackADDR();

    uint8_t res = mem[addr];
    Cycles--;
    printf("Popping %02X from stack addr %04x\n", res, StackADDR());
    SP++;
    return res;
}

uint16_t CPU::PopW()
{
    uint16_t res = read16(StackADDR() + 1);
    SP += 2;
    Cycles--;
    printf("Popping %04X from stack addr %04x\n", res, StackADDR() - 1);
    
    return res;
}

void CPU::SetFlag(flags flag, bool val)
{
    if (val)
        status |= flag;
    else
        status &= ~flag;
}

bool CPU::GetFlag(flags flag)
{
    return (status & flag) != 0;
}

#ifdef USE_ADDRMODES

// Addressing Modes
uint16_t CPU::Addr_ZP0() // Zero Page
{
    uint16_t addr = FetchB();
    return addr;
}

uint16_t CPU::Addr_ZPX() // Zero Page X
{
    uint16_t addr = FetchB();
    addr += X;
    return addr;
}

uint16_t CPU::Addr_ZPY() // Zero Page Y
{
    uint16_t addr = FetchB();
    addr += Y;
    return addr;
}

uint16_t CPU::Addr_IZX() // Indirect X
{
    uint16_t addr = FetchB();
    addr += X;
    addr = ReadW(addr);
    return addr;
}

uint16_t CPU::Addr_IZY() // Indirect Y
{
    uint16_t addr = FetchB();
    addr = ReadW(addr);
    addr += Y;
    return addr;
}

uint16_t CPU::Addr_ABS() // Absolute
{
    uint16_t addr = FetchW();
    return addr;
}

uint16_t CPU::Addr_ABX() // Absolute X
{
    uint16_t addr = FetchW();
    addr += X;
    return addr;
}

uint16_t CPU::Addr_ABY() // Absolute Y
{
    uint16_t addr = FetchW();
    addr += Y;
    return addr;
}

uint16_t CPU::Addr_IND() // Indirect
{
    uint16_t addr = FetchW();
    uint16_t rrrrrrr = addr;
    addr = ReadW(addr);
    printf("0x%04x --> 0x%04x\n", rrrrrrr, addr);
    return addr;
}

uint16_t CPU::Addr_REL() // Relative
{
    uint16_t addr = FetchB();
    if (addr & 0x80)
        addr |= 0xFF00;
    addr += PC;
    return addr;
}

#endif

void CPU::Execute(uint32_t cycles)
{
    this->Cycles = cycles;

    uint8_t instruction = FetchB();
    uint8_t opcode = instruction & 0xF0;
    Instruction ins;

    uint16_t addr;
    uint8_t val;
    uint16_t res;
    uint16_t addr_abs;

    switch (instruction)
    {
    case ins.LDA:
        switch(AM)
        {
        case AM_IMM:
            A = FetchB();
            printf("LDA IMM");
            break;
        case AM_ZP0:
            addr = Addr_ZP0();
            A = ReadB(addr);
            printf("LDA ZP0");
            break;
        case AM_ZPX:
            addr = Addr_ZPX();
            A = ReadB(addr);
            printf("LDA ZPX");
            break;
        case AM_ZPY:
            addr = Addr_ZPY();
            A = ReadB(addr);
            printf("LDA ZPY");
            break;
        case AM_ABS:
            addr = Addr_ABS();
            A = ReadB(addr);
            printf("LDA ABS");
            break;
        case AM_ABX:
            addr = Addr_ABX();
            A = ReadB(addr);
            printf("LDA ABX");
            break;
        case AM_ABY:
            addr = Addr_ABY();
            A = ReadB(addr);
            printf("LDA ABY");
            break;
        case AM_IND:
            addr = Addr_IND();
            A = ReadB(addr);
            printf("LDA IND");
            break;
        case AM_INX:
            addr = Addr_IZX();
            A = ReadB(addr);
            printf("LDA INX");
            break;
        case AM_INY:
            addr = Addr_IZY();
            A = ReadB(addr);
            printf("LDA INY");
            break;
        }
        SetFlag(Z, A == 0);
        SetFlag(N, A & 0x80);
        break;
    case ins.LDX:
        switch(AM)
        {
        case AM_IMM:
            X = FetchB();
            printf("LDX IMM");
            break;
        case AM_ZP0:
            addr = Addr_ZP0();
            X = ReadB(addr);
            printf("LDX ZP0");
            break;
        case AM_ZPY:
            addr = Addr_ZPY();
            X = ReadB(addr);
            printf("LDX ZPY");
            break;
        case AM_ABS:
            addr = Addr_ABS();
            X = ReadB(addr);
            printf("LDX ABS");
            break;
        case AM_ABY:
            addr = Addr_ABY();
            X = ReadB(addr);
            printf("LDX ABY");
            break;
        case AM_IND:
            addr = Addr_IND();
            X = ReadB(addr);
            printf("LDX IND");
            break;
        case AM_INY:
            addr = Addr_IZX();
            X = ReadB(addr);
            printf("LDX INY");
            break;
        }
        SetFlag(Z, X == 0);
        SetFlag(N, X & 0x80);
        break;
    case ins.LDY:
        switch(AM)
        {
        case AM_IMM:
            Y = FetchB();
            printf("LDY IMM");
            break;
        case AM_ZP0:
            addr = Addr_ZP0();
            Y = ReadB(addr);
            printf("LDY ZP0");
            break;
        case AM_ZPX:
            addr = Addr_ZPX();
            Y = ReadB(addr);
            printf("LDY ZPX");
            break;
        case AM_ABS:
            addr = Addr_ABS();
            Y = ReadB(addr);
            printf("LDY ABS");
            break;
        case AM_ABX:
            addr = Addr_ABX();
            Y = ReadB(addr);
            printf("LDY ABX");
            break;
        case AM_IND:
            addr = Addr_IND();
            Y = ReadB(addr);
            printf("LDY IND");
            break;
        case AM_INX:
            addr = Addr_IZX();
            Y = ReadB(addr);
            printf("LDY INX");
            break;
        }
        SetFlag(Z, Y == 0);
        SetFlag(N, Y & 0x80);
        break;
    case ins.STA:
        switch(AM)
        {
        case AM_ZP0:
            addr = Addr_ZP0();
            WriteB(addr, A);
            printf("STA ZP0");
            break;
        case AM_ZPX:
            addr = Addr_ZPX();
            WriteB(addr, A);
            printf("STA ZPX");
            break;
        case AM_ZPY:
            addr = Addr_ZPY();
            WriteB(addr, A);
            printf("STA ZPY");
            break;
        case AM_ABS:
            addr = Addr_ABS();
            WriteB(addr, A);
            printf("STA ABS");
            break;
        case AM_ABX:
            addr = Addr_ABX();
            WriteB(addr, A);
            printf("STA ABX");
            break;
        case AM_ABY:
            addr = Addr_ABY();
            WriteB(addr, A);
            printf("STA ABY");
            break;
        case AM_IND:
            addr = Addr_IND();
            WriteB(addr, A);
            printf("STA IND");
            break;
        case AM_INX:
            addr = Addr_IZX();
            WriteB(addr, A);
            printf("STA INX");
            break;
        case AM_INY:
            addr = Addr_IZY();
            WriteB(addr, A);
            printf("STA INY");
            break;
        }
        break;
    case ins.STX:
        switch(AM)
        {
        case AM_ZP0:
            addr = Addr_ZP0();
            WriteB(addr, X);
            printf("STX ZP0");
            break;
        case AM_ZPY:
            addr = Addr_ZPY();
            WriteB(addr, X);
            printf("STX ZPY");
            break;
        case AM_ABS:
            addr = Addr_ABS();
            WriteB(addr, X);
            printf("STX ABS");
            break;
        case AM_ABY:
            addr = Addr_ABY();
            WriteB(addr, X);
            printf("STX ABY");
            break;
        case AM_IND:
            addr = Addr_IND();
            WriteB(addr, X);
            printf("STX IND");
            break;
        case AM_INY:
            addr = Addr_IZX();
            WriteB(addr, X);
            printf("STX INY");
            break;
        }
        break;
    case ins.STY:
        switch(AM)
        {
        case AM_ZP0:
            addr = Addr_ZP0();
            WriteB(addr, Y);
            printf("STY ZP0");
            break;
        case AM_ZPX:
            addr = Addr_ZPX();
            WriteB(addr, Y);
            printf("STY ZPX");
            break;
        case AM_ABS:
            addr = Addr_ABS();
            WriteB(addr, Y);
            printf("STY ABS");
            break;
        case AM_ABX:
            addr = Addr_ABX();
            WriteB(addr, Y);
            printf("STY ABX");
            break;
        case AM_IND:
            addr = Addr_IND();
            WriteB(addr, Y);
            printf("STY IND");
            break;
        case AM_INX:
            addr = Addr_IZX();
            WriteB(addr, Y);
            printf("STY INX");
            break;
        }
        break;
    case ins.TAX:
        X = A;
        printf("TAX");
        SetFlag(Z, X == 0);
        SetFlag(N, X & 0x80);
        break;
    case ins.TAY:
        Y = A;
        printf("TAY");
        SetFlag(Z, Y == 0);
        SetFlag(N, Y & 0x80);
        break;
    case ins.TSX:
        X = SP;
        printf("TSX");
        SetFlag(Z, X == 0);
        SetFlag(N, X & 0x80);
        break;
    case ins.TXA:
        A = X;
        printf("TXA");
        SetFlag(Z, A == 0);
        SetFlag(N, A & 0x80);
        break;
    case ins.TXS:
        SP = X;
        printf("TXS");
        break;
    case ins.TYA:
        A = Y;
        printf("TYA");
        SetFlag(Z, A == 0);
        SetFlag(N, A & 0x80);
        break;
    case ins.ADD:
        switch(AM)
        {
        case AM_IMM:
            tmp = FetchB();
            printf("ADD IMM");
            break;
        case AM_ZP0:
            addr = Addr_ZP0();
            tmp = ReadB(addr);
            printf("ADD ZP0");
            break;
        case AM_ZPX:
            addr = Addr_ZPX();
            tmp = ReadB(addr);
            printf("ADD ZPX");
            break;
        case AM_ABS:
            addr = Addr_ABS();
            tmp = ReadB(addr);
            printf("ADD ABS");
            break;
        case AM_ABX:
            addr = Addr_ABX();
            tmp = ReadB(addr);
            printf("ADD ABX");
            break;
        case AM_ABY:
            addr = Addr_ABY();
            tmp = ReadB(addr);
            printf("ADD ABY");
            break;
        case AM_IND:
            addr = Addr_IND();
            tmp = ReadB(addr);
            printf("ADD IND");
            break;
        case AM_INX:
            addr = Addr_IZX();
            tmp = ReadB(addr);
            printf("ADD INX");
            break;
        case AM_INY:
            addr = Addr_IZY();
            tmp = ReadB(addr);
            printf("ADD INY");
            break;
        }
        A += tmp;
        SetFlag(C, A & 0x100);
        SetFlag(Z, A == 0);
        SetFlag(N, A & 0x80);
        break;
    case ins.ADC:
        switch(AM)
        {
        case AM_IMM:
            tmp = FetchB();
            printf("ADC IMM");
            break;
        case AM_ZP0:
            addr = Addr_ZP0();
            tmp = ReadB(addr);
            printf("ADC ZP0");
            break;
        case AM_ZPX:
            addr = Addr_ZPX();
            tmp = ReadB(addr);
            printf("ADC ZPX");
            break;
        case AM_ABS:
            addr = Addr_ABS();
            tmp = ReadB(addr);
            printf("ADC ABS");
            break;
        case AM_ABX:
            addr = Addr_ABX();
            tmp = ReadB(addr);
            printf("ADC ABX");
            break;
        case AM_ABY:
            addr = Addr_ABY();
            tmp = ReadB(addr);
            printf("ADC ABY");
            break;
        case AM_IND:
            addr = Addr_IND();
            tmp = ReadB(addr);
            printf("ADC IND");
            break;
        case AM_INX:
            addr = Addr_IZX();
            tmp = ReadB(addr);
            printf("ADC INX");
            break;
        case AM_INY:
            addr = Addr_IZY();
            tmp = ReadB(addr);
            printf("ADC INY");
            break;
        }
        tmp += A + GetFlag(C);
        SetFlag(C, tmp & 0x100);
        SetFlag(Z, tmp == 0);
        SetFlag(N, tmp & 0x80);
        A = tmp;
        break;
    case ins.SUB:
        switch(AM)
        {
        case AM_IMM:
            tmp = FetchB();
            break;
        case AM_ZP0:
            addr = Addr_ZP0();
            tmp = ReadB(addr);
            break;
        case AM_ZPX:
            addr = Addr_ZPX();
            tmp = ReadB(addr);
            break;
        case AM_ABS:
            addr = Addr_ABS();
            tmp = ReadB(addr);
            break;
        case AM_ABX:
            addr = Addr_ABX();
            tmp = ReadB(addr);
            break;
        case AM_ABY:
            addr = Addr_ABY();
            tmp = ReadB(addr);
            break;
        case AM_IND:
            addr = Addr_IND();
            tmp = ReadB(addr);
            break;
        case AM_INX:
            addr = Addr_IZX();
            tmp = ReadB(addr);
            break;
        case AM_INY:
            addr = Addr_IZY();
            tmp = ReadB(addr);
            break;
        }
        tmp = A - tmp;
        SetFlag(C, tmp & 0x100);
        SetFlag(Z, tmp == 0);
        SetFlag(V, (A ^ tmp) & (A ^ tmp) & 0x80);
        SetFlag(N, tmp & 0x80);
        A = tmp;
        printf("SUB");
        break;
    case ins.SBC:
        switch(AM)
        {
        case AM_IMM:
            tmp = FetchB();
            break;
        case AM_ZP0:
            addr = Addr_ZP0();
            tmp = ReadB(addr);
            break;
        case AM_ZPX:
            addr = Addr_ZPX();
            tmp = ReadB(addr);
            break;
        case AM_ABS:
            addr = Addr_ABS();
            tmp = ReadB(addr);
            break;
        case AM_ABX:
            addr = Addr_ABX();
            tmp = ReadB(addr);
            break;
        case AM_ABY:
            addr = Addr_ABY();
            tmp = ReadB(addr);
            break;
        case AM_IND:
            addr = Addr_IND();
            tmp = ReadB(addr);
            break;
        case AM_INX:
            addr = Addr_IZX();
            tmp = ReadB(addr);
            break;
        case AM_INY:
            addr = Addr_IZY();
            tmp = ReadB(addr);
            break;
        }
        tmp = A - tmp - GetFlag(C);
        SetFlag(C, tmp & 0x100);
        SetFlag(Z, tmp == 0);
        SetFlag(V, (A ^ tmp) & (A ^ tmp) & 0x80);
        SetFlag(N, tmp & 0x80);
        A = tmp;
        printf("SBC");
        break;
    case ins.AND:
        switch(AM)
        {
        case AM_IMM:
            tmp = FetchB();
            break;
        case AM_ZP0:
            addr = Addr_ZP0();
            tmp = ReadB(addr);
            break;
        case AM_ZPX:
            addr = Addr_ZPX();
            tmp = ReadB(addr);
            break;
        case AM_ABS:
            addr = Addr_ABS();
            tmp = ReadB(addr);
            break;
        case AM_ABX:
            addr = Addr_ABX();
            tmp = ReadB(addr);
            break;
        case AM_ABY:
            addr = Addr_ABY();
            tmp = ReadB(addr);
            break;
        case AM_IND:
            addr = Addr_IND();
            tmp = ReadB(addr);
            break;
        case AM_INX:
            addr = Addr_IZX();
            tmp = ReadB(addr);
            break;

        case AM_INY:
            addr = Addr_IZY();
            tmp = ReadB(addr);
            break;
        }
        tmp &= A;
        SetFlag(Z, tmp == 0);
        SetFlag(N, tmp & 0x80);
        A = tmp;
        printf("AND");
        break;
    case ins.OR:
        switch (AM)
        {
        case AM_IMM:
            tmp = FetchB();
            break;
        case AM_ZP0:
            addr = Addr_ZP0();
            tmp = ReadB(addr);
            break;
        case AM_ZPX:
            addr = Addr_ZPX();
            tmp = ReadB(addr);
            break;
        case AM_ABS:
            addr = Addr_ABS();
            tmp = ReadB(addr);
            break;
        case AM_ABX:
            addr = Addr_ABX();
            tmp = ReadB(addr);
            break;
        case AM_ABY:
            addr = Addr_ABY();
            tmp = ReadB(addr);
            break;
        case AM_IND:
            addr = Addr_IND();
            tmp = ReadB(addr);
            break;
        case AM_INX:
            addr = Addr_IZX();
            tmp = ReadB(addr);
            break;

        case AM_INY:
            addr = Addr_IZY();
            tmp = ReadB(addr);
            break;
        }
        tmp |= A;
        SetFlag(Z, tmp == 0);
        SetFlag(N, tmp & 0x80);
        A = tmp;
        printf("OR");
        break;
    case ins.NOR:
        switch (AM)
        {
        case AM_IMM:
            tmp = FetchB();
            break;
        case AM_ZP0:
            addr = Addr_ZP0();
            tmp = ReadB(addr);
            break;
        case AM_ZPX:
            addr = Addr_ZPX();
            tmp = ReadB(addr);
            break;
        case AM_ABS:
            addr = Addr_ABS();
            tmp = ReadB(addr);
            break;
        case AM_ABX:
            addr = Addr_ABX();
            tmp = ReadB(addr);
            break;
        case AM_ABY:
            addr = Addr_ABY();
            tmp = ReadB(addr);
            break;
        case AM_IND:
            addr = Addr_IND();
            tmp = ReadB(addr);
            break;
        case AM_INX:
            addr = Addr_IZX();
            tmp = ReadB(addr);
            break;

        case AM_INY:
            addr = Addr_IZY();
            tmp = ReadB(addr);
            break;
        }
        tmp = ~(A | tmp);
        SetFlag(Z, tmp == 0);
        SetFlag(N, tmp & 0x80);
        A = tmp;
        printf("NOR");
        break;
    case ins.CMP:
        switch (AM)
        {
        case AM_IMM:
            tmp = FetchB();
            break;
        case AM_ZP0:
            addr = Addr_ZP0();
            tmp = ReadB(addr);
            break;
        case AM_ZPX:
            addr = Addr_ZPX();
            tmp = ReadB(addr);
            break;
        case AM_ABS:
            addr = Addr_ABS();
            tmp = ReadB(addr);
            break;
        case AM_ABX:
            addr = Addr_ABX();
            tmp = ReadB(addr);
            break;
        case AM_ABY:
            addr = Addr_ABY();
            tmp = ReadB(addr);
            break;
        case AM_IND:
            addr = Addr_IND();
            tmp = ReadB(addr);
            break;
        case AM_INX:
            addr = Addr_IZX();
            tmp = ReadB(addr);
            break;

        case AM_INY:
            addr = Addr_IZY();
            tmp = ReadB(addr);
            break;
        }
        res = A - tmp;
        SetFlag(C, A >= tmp);
        SetFlag(Z, tmp == A);
        SetFlag(V, (A ^ tmp) & (A ^ tmp) & 0x80);
        SetFlag(N, tmp & 0x80);
        printf("CMP");
        break;
    case ins.JNZ:
        if(!GetFlag(Z))
        {
            addr_abs = FetchW();
            PC = addr_abs;
        }
        else
        {
            PC += 2;
        }
        printf("JNZ");
        break;
    case ins.JZ:
        if(GetFlag(Z))
        {
            addr_abs = FetchW();
            PC = addr_abs;
        }
        else
        {
            PC += 2;
        }
        printf("JZ");
        break;
    case ins.JMP:
        addr_abs = FetchW();
        PC = addr_abs;
        printf("JMP");
        break;
    case ins.JSR:
        addr_abs = FetchW();
        PushW(PC - 1);
        PC = addr_abs;
        printf("JSR");
        break;
    case ins.RET:
        PC = PopW();
        printf("RET");
        break;
    case ins.IRET:
        status = PopB();
        PC = PopW();
        printf("IRET");
        break;
    case ins.INP:
        PC++;
        printf("INP");
        break;
    case ins.AP2:
        PC++;
        PC++;
        printf("AP2");
        break;
    case ins.PHA:
    {
        PushB(A);
        printf("PHA");
    }break;
    case ins.PLA:
        A = PopB();
        SetFlag(Z, A == 0);
        SetFlag(N, A & 0x80);
        printf("PLA");
        break;
    case ins.PHP:
        PushB(status | 0x30);
        printf("PHP");
        break;
    case ins.PLP:
        status = PopB() | 0x20;
        printf("PLP");
        break;
    case 0x00:
        printf("NOP");
        break;
    case ins.CHAM:
        AM = FetchB();
        printf("CHAM");
        break;
    case ins.EXT:
        printf("EXT");
        return;
        break;
    case ins.INX:
        X++;
        SetFlag(Z, X == 0);
        SetFlag(N, X & 0x80);
        cycles--;
        printf("INX");
        break;
    case ins.INY:
        Y++;
        SetFlag(Z, Y == 0);
        SetFlag(N, Y & 0x80);
        cycles--;
        printf("INY");
        break;
    case ins.DEX:
        X--;
        SetFlag(Z, X == 0);
        SetFlag(N, X & 0x80);
        cycles--;
        printf("DEX");
        break;
    case ins.DEY:
        Y--;
        SetFlag(Z, Y == 0);
        SetFlag(N, Y & 0x80);
        cycles--;
        printf("DEY");
        break;
    case ins.CPX:
        switch (AM)
        {
        case AM_IMM:
            tmp = FetchB();
            break;
        case AM_ZP0:
            addr = Addr_ZP0();
            tmp = ReadB(addr);
            break;
        case AM_ZPX:
            addr = Addr_ZPX();
            tmp = ReadB(addr);
            break;
        case AM_ABS:
            addr = Addr_ABS();
            tmp = ReadB(addr);
            break;
        case AM_ABX:
            addr = Addr_ABX();
            tmp = ReadB(addr);
            break;
        case AM_ABY:
            addr = Addr_ABY();
            tmp = ReadB(addr);
            break;
        case AM_IND:
            addr = Addr_IND();
            tmp = ReadB(addr);
            break;
        case AM_INX:
            addr = Addr_IZX();
            tmp = ReadB(addr);
            break;

        case AM_INY:
            addr = Addr_IZY();
            tmp = ReadB(addr);
            break;
        }
        res = X - tmp;
        SetFlag(C, X >= tmp);
        SetFlag(Z, tmp == X);
        SetFlag(V, (X ^ tmp) & (X ^ tmp) & 0x80);
        SetFlag(N, tmp & 0x80);
        printf("CPX");
        break;
    case ins.CPY:
        switch (AM)
        {
        case AM_IMM:
            tmp = FetchB();
            break;
        case AM_ZP0:
            addr = Addr_ZP0();
            tmp = ReadB(addr);
            break;
        case AM_ZPX:
            addr = Addr_ZPX();
            tmp = ReadB(addr);
            break;
        case AM_ABS:
            addr = Addr_ABS();
            tmp = ReadB(addr);
            break;
        case AM_ABX:
            addr = Addr_ABX();
            tmp = ReadB(addr);
            break;
        case AM_ABY:
            addr = Addr_ABY();
            tmp = ReadB(addr);
            break;
        case AM_IND:
            addr = Addr_IND();
            tmp = ReadB(addr);
            break;
        case AM_INX:
            addr = Addr_IZX();
            tmp = ReadB(addr);
            break;

        case AM_INY:
            addr = Addr_IZY();
            tmp = ReadB(addr);
            break;
        }
        res = Y - tmp;
        SetFlag(C, Y >= tmp);
        SetFlag(Z, tmp == Y);
        SetFlag(V, (Y ^ tmp) & (Y ^ tmp) & 0x80);
        SetFlag(N, tmp & 0x80);
        printf("CPY");
        break;
    default:
        printf("Instruction 0x%02x not implemented. PC is 0x%04x\n", instruction, PC);
        break;
    }
    printf("\nIns: 0x%02x, REG_A: 0x%02x, REG_X: 0x%02x, REG_Y: 0x%02x, REG_AM: 0x%02x, PC: 0x%04x, SP: 0x%02x\nStack: |%02x|%02x|%02x|%02x|%02x|%02x|%02x|%02x|%02x|%02x|%02x|%02x|%02x|%02x|%02x|%02x|, Status: |%s|%s|%s|%s|%s|%s|\n", instruction, A, X, Y, AM, PC, SP, mem[(SP_W_ADDR | 0xFF)], mem[(SP_W_ADDR | 0xFE)], mem[(SP_W_ADDR | 0xFD)], mem[(SP_W_ADDR | 0xFC)], mem[(SP_W_ADDR | 0xFB)], mem[(SP_W_ADDR | 0xFA)], mem[(SP_W_ADDR | 0xF9)], mem[(SP_W_ADDR | 0xF8)], mem[(SP_W_ADDR | 0xF7)], mem[(SP_W_ADDR | 0xF6)], mem[(SP_W_ADDR | 0xF5)], mem[(SP_W_ADDR | 0xF4)], mem[(SP_W_ADDR | 0xF3)], mem[(SP_W_ADDR | 0xF2)], mem[(SP_W_ADDR | 0xF1)], mem[(SP_W_ADDR | 0xF0)], GetFlag(C) ? "C" : "c", GetFlag(Z) ? "Z" : "z", GetFlag(I) ? "I" : "i", GetFlag(B) ? "B" : "b", GetFlag(V) ? "V" : "v", GetFlag(N) ? "N" : "n");
}

void CPU::IRQ()
{
    if(!GetFlag(I))
    {
        PushW(PC);
        PushB(status);
        SetFlag(B, true);
        SetFlag(I, true);
        PC = ReadW(0xFFFE);
    }
}

void CPU::NMI()
{
    PushW(PC);
    PushB(status);
    SetFlag(B, true);
    SetFlag(I, true);
    PC = ReadW(0xFFFA);
}

void CPU::step()
{
    Execute(1);
}

void CPU::Dump(char* msg)
{
    printf("Dumping CPU state and memory to cpu.dump\n");
    FILE* f = fopen("cpu.dump", "w");
    fprintf(f, "---------------------------------------------------------------------------------------------\n");
    fprintf(f, "%s\n", msg);
    fprintf(f, "A: %02x\n", A);
    fprintf(f, "X: %02x\n", X);
    fprintf(f, "Y: %02x\n", Y);
    fprintf(f, "SP: 0x%04x\n", SP);
    fprintf(f, "stack: Starting Address is 0x%04x, size is %s. Stack Contents:\n", SP_W_ADDR | SP_START, strndup(byte(SP_START - SP_END), strlen(byte(SP_START - SP_END)) - 1));
    for(int i = SP_END; i != SP_START + 1; i++)
    {
        if(i % 16 == 0)
            fprintf(f, "%04x: ", i);
        fprintf(f, "%02x ", mem[SP_W_ADDR | i]);
        if(i % 16 == 15)
            fprintf(f, "\n");
    }
    fprintf(f, "\n");
    fprintf(f, "PC: %04x\n", PC);
    fprintf(f, "Status: %02x\n", status);
    fprintf(f, "Cycles: %d\n\n", Cycles);
    fprintf(f, "Memory:\n");
    for(int i = 0; i < 0xFFFF + 1; i++)
    {
        if(i % 16 == 0)
            fprintf(f, "%04x: ", i);
        fprintf(f, "%02x ", mem[i]);
        if(i % 16 == 15)
            fprintf(f, "\n");
    }
    fprintf(f, "\n---------------------------------------------------------------------------------------------\n");
    fclose(f);
}

void CPU::run(int freq)
{
    int cycles = 0;
    char c;
    printf("Running CPU at %dHz\n", freq);
    while(c != 'q')
    {
        for(int i = 0; i < freq; i++)
        {
            step();
            cycles++;

            if(cycles % 1000000 == 0)
                printf("Cycles: %d\n", cycles);
        }
        sleep(1);
    }
}

System::System()
{
    cpu = new CPU();
    instance = this;
}

void System::run()
{
    cpu->run(CPU_CLOCK_FREQUENCY);
}

void System::Reset()
{
    cpu->ResetCPU();
}

void System::load(char* filename)
{
    FILE* f = fopen(filename, "rb");
    if(f == NULL)
    {
        printf("Error opening file %s\n", filename);
        return;
    }

    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t *data;
    fread(data, 1, size, f);
    fclose(f);

    for(int i = 0; i < size; i++)
        cpu->write(i, data[i]);
}

void System::load(uint8_t* data, uint32_t size)
{
    for(int i = 0; i < size; i++)
        cpu->write(i, data[i]);
}

void System::save(char* filename)
{
    FILE* f = fopen(filename, "wb");
    if(f == NULL)
    {
        printf("Error opening file %s\n", filename);
        return;
    }

    uint8_t *data = new uint8_t[0xFFFF];
    for(int i = 0; i < 0xFFFF; i++)
        data[i] = cpu->read(i);

    fwrite(data, 1, 0xFFFF, f);
    fclose(f);
}

void System::dump()
{
    cpu->Dump("SYS DUMP");
}

void System::step()
{
    cpu->step();
}

uint8_t CPU::read(uint16_t addr)
{
    return mem[addr];
}

void CPU::ChangeREG(int reg, uint8_t val)
{
    switch(reg)
    {
    case 0:
        A = val;
        break;
    case 1:
        X = val;
        break;
    case 2:
        Y = val;
        break;
    }
}

uint16_t CPU::read16(uint16_t addr)
{
    return mem[addr] | (mem[addr + 1] << 8);
}

void CPU::write(uint16_t addr, uint8_t data)
{
    mem[addr] = data;
}

void CPU::write16(uint16_t addr, uint16_t data)
{
    mem[addr] = data & 0xFF;
    mem[addr + 1] = (data >> 8) & 0xFF;
}

void System::change_reg(int reg, uint8_t val)
{
    cpu->ChangeREG(reg, val);
}