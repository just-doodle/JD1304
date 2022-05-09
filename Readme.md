# JD1304

A 8-bit microcomputer emulator with a a custom-designed processor.

2nd rewrite of the discontinued JD1618.

## Specifications

* 16 bits Address Bus
* 8 bits Data Bus
* 64K RAM
* 4 registers: A, X, Y, AM
* 37 instructions
* 10 Addressing Modes

## Building

To build the project, run the following command:

```bash
cd JD1304
make JD1304
```

To run the project, run the following command:

```bash
cd JD1304
make run
```

To install the project, run the following command:

```bash
cd JD1304
make install
```

There is a sample program in the `examples` directory.

## Registers

```Text
A: Accumulator - The accumulator is a register that holds the result of an operation.
X: X Index - The X index is a register that holds the X index of the current address.
Y: Y Index - The Y index is a register that holds the Y index of the current address.
AM: Addressing Mode - The AM is a register that is used to change the Addressing mode of current instruction.
PC: Program Counter - The program counter is a register that holds the current address.
SP: Stack Pointer - The stack pointer is a register that holds the current stack pointer.
SR: Status Register - The status register is a register that holds the status of the processor.
```

## Addressing Modes

```Text
IMM: Immediate - The operand is the next byte in the program.
ZP: Zero Page - The operand is the next byte in the program, but is stored in the zero page.
ZPX: Zero Page Indexed - The operand is the next byte in the program, but is stored in the zero page, and is indexed by the X index.
ZPY: Zero Page Indexed - The operand is the next byte in the program, but is stored in the zero page, and is indexed by the Y index.
ABS: Absolute - The operand is the next two bytes in the program.
ABSX: Absolute Indexed - The operand is the next two bytes in the program, and is indexed by the X index.
ABSY: Absolute Indexed - The operand is the next two bytes in the program, and is indexed by the Y index.
IND: Indirect - The next two bytes in the program are the pointer to the operand.
INDX: Indirect Indexed - The next two bytes in the program are the pointer to the operand, and is indexed by the X index.
INDY: Indirect Indexed - The next two bytes in the program are the pointer to the operand, and is indexed by the Y index.
IMP: Implied - The operand is implied.
```

## Status Flags

```Text
N: Negative - The negative flag is set if the result is negative.
V: Overflow - The overflow flag is set if the result is too large to fit in 8 bits.
B: Borrow - The borrow flag is set if the result is too large to fit in 8 bits.
C: Carry - The carry flag is set if the result is too large to fit in 8 bits.
Z: Zero - The zero flag is set if the result is zero.
I: Interrupt - If the interrupt flag is set, the processor will be interrupted with an interrupt request.
```

## TODO

* Communication with the devices with the ports and INB/OUTB instructions.
* Interrupts.
* Assembly language.
* Assembler
* Disassembler
* Modules
* Refinement of the instructions.
  