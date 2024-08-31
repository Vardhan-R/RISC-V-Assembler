addi x1 x1 0
sd x2 0(x3)
lui x3 1000000
lui x3 -1
lui x3 -2
auipc x3 65536
auipc x3 0x10000
L1: add x0 x0 x0
beq x0 x0 L1
jal x0 L1
beq x0 x0 L1
