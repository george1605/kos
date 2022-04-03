global int10h ;just some assembly
global int13h
global int17h
global int14h
global bios_start
global eficall0

bios_start:
  mov ax,0
  mov bx,0
  call bios_main ;defined in bios.h

int10h:
  ret
  
int13h:
  ret
  
int17h:
  ret

eficall0:
  subq $40, rsp
	call *rdi
	addq $40, rsp
	ret
