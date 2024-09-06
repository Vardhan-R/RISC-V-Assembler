output: main.o RISCV_instr_constants.o
	g++ main.o RISCV_instr_constants.o -o riscv_asm

main.o: main.cpp
	g++ -c main.cpp

RISCV_instr_constants.o: RISCV_instr_constants.cpp
	g++ -c RISCV_instr_constants.cpp

clean:
	rm *.o riscv_asm *.hex
