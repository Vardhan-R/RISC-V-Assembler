# RISC-V-Assembler
A RISC-V assembler (RV64).

## To compile
`make`
- to compile run: `make`
- to run: `./riscv_asm.exe <input file>`
- to clean object files: `make clean`

## Debugging
- `-d`: display hex code on terminal
- `-l`: display labels on terminal
- `-o`: display parsed tokens on terminal

##  Features
1. **Implemented handling of blank lines and comments in source files** (`;`, `#`). It can also ignore `.text`, `.word`, etc., type instructions. It is also robust towards variable blank spaces and commas between instructions.

2. **Can accept immediate values** in the form of decimal, binary (starting with `0b`, e.g., `0b100`), and hexadecimal format (starting with `0x`, e.g., `0x1fa`).

3. **Implemented various flags for debug purposes**, along with the source code as input. One can use the following flags to see output on the terminal:

    - `-d`: prints machine code in terminal
    - `-l`: prints the labels along with line number (PC value at label)
    - `-o`: prints the parsed tokens from instructions separated by “-”

4. **Error handling**: If the program encounters an error in the source code, it can pinpoint the line number and display relevant messages. The following errors are checked:

    - Type of instruction is a valid type or not
    - Number of arguments for a particular instruction type
    - Correctness of register aliases and checks  registers are within range
    - Ensures immediate values are decimal, hexadecimal or binary format
    - Ensures labels are properly defined and not repeated
    - The immediate values are within the range specified by ISA
        (generates a warning when the immediate value is out of range but only condsiders the expected number of bits spcified by the ISA like I type extracts 12 bits.)

## Limitations (cannot handle)
- ecall
- ebreak
- Relative jump with numbers as immediate instead of labels (B-type and J-type)
- I type instructions specified in the format `ld rd imm rs1`
- S type instructions specified in the format `sd rs1 imm rd`
- evaluation of mathematical expressions as `imm`

## Known errors 
- Parses `ld rd rs1 imm` for I type instruction. (syntactically incorrect but our program parses it without throwing error)
- Parses `jalr rd rs1 offset`.



