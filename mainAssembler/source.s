.data
    .dword 28 37

.text
    addi x0 x0 1
    ld x0 1(x1)
    ld x0 1 x1
    jalr x0 x1 1
    addi x9 x9(1)
    ld x9 1 x9

    addi x15, x22, -45
    and x23, x8, x9
    blt x2, x11, 240
    sd x19, -54(x1)
    jal x3, -10116
