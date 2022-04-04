run: ccomp
  nasm -f elf32 -o loader.o boot.asm

ccomp:
  gcc -m32 -o kernel.o -c kernel.c -nostdlib -nostartfiles -nodefaultlibs
