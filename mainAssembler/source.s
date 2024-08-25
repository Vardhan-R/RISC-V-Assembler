.data
    .dword 28 37

.text
    addi x0 x0 1
    ld x0 1(x1)
    ld x0 1 x1
    jalr x0 x1 1
    addi x9 x9(1)
    ld x9 1 x9 
