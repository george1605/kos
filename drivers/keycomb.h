#include "../lib.c"
struct skeys {
  int shift;
  int alt;
  int comm; // command or control or whatever
};

void printkey(int key,struct skeys u){
  if(key <= 'z' && key >= 'a' && u.shift) 
    kputc(key + 32);
}
