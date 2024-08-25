doesnt handle ecall ebreak
--------------------------------
cant handle smtg like 
L1:
addi x1, x1, 1

cant have .text , .word ...
-------------------------------
must follow 

If it helps, you may assume that there is only 1 space in between the instruction and the
first operand. Also, only 1 space between “,” and second operand and so on. Similarly,
one colon after the label and then one space.

There are no blank lines in the input file
--------------------------------

make
to compile run: make
to run: ./main.exe <input file> 
to clean object files: make clean
