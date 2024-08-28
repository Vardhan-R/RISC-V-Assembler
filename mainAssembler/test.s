.data
    .dword 28 37

.text
    add ra zero s3
    sub a0 t6 t1
    xor gp ra sp
       or t0 t1 x9
      and x0 x1 x2
    sll a4 x14 x31
    srl a7 a7 a7
    sra s0 s6 s5
    slt x9 x9 x8
            sltu x20 x18 s11   
    
    addi s4 x31 -8   # rijw  uefr  
    xori a0 tp 283
    ori x0 a1 -1
         srai x1 x11 28
    
    sb x1 0(x2)
    sw x7 19(x2)
    sd x9 99(x13)
    sh x8 -1(x31)
    
    lui x7 1010000
     auipc x3 1010101
    
    Exit:
        add x0 x0 x0

    lmao: add x0 x0 x0
    
    pink:

        add x0 x0 x0
        
    l1:        add x0 x1 x1
    l3: 
           add x0 x1 x1
    l2:        

    add x0 x1 x1

    beq x0 zero l1
