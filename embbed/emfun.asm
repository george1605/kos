section .bootloader
    global load
    global quit

load:
    push r1
    push r2
    ret ; returns to C code

quit:
    hlt
    jmp quit
