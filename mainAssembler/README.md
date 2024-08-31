# Main Assembler

## Doesn't handle
- ecall
- ebreak
- rel jmp (B-type)
- rel jmp (J-type)
- `ld rd imm rs1`
- `sd rs1 imm rd`
- evaluation of mathematical expressions as `imm`

## Error handling (done)
- check register names and aliases are valid
- check if the instruction is valid
- check if the immediate value is valid
- check if the label is valid

## To compile
`make`
- to compile run: `make`
- to run: `./main.exe <input file>`
- to clean object files: `make clean`

## Debugging
- `-d`: display hex code on terminal
- `-l`: display labels on terminal
- `-o`: display parsed tokens on terminal

## Limitations
- Parses `ld rd rs1 imm`
