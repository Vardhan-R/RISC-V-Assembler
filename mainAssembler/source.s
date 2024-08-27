beq x1, x2, L1
jal x0, L2 # comment
L1: xor  a5, a3, a7
lui x9, 0x10000 # helllo
L2: add t1, x8, s10
jalr x0, 0(x1)