#pragma once
#include "../stdlib.c"
static char memory[30000];
static char* filename = "/home/bf/output.txt";
int cell = 0, bracket_flag = 0; 

// writes character to file 
FILE* bffile;

static void bfput(char c)
{
    fputc(c, bffile);
}

void bfexec(char* s) {
  char p[1001], c = 0;
  memcpy(p,s,strlen(s));
  bffile = fopen(filename, "w");
  while(p[c]){
    switch(p[c]){
     case '+':
      memory[cell]++; break;
     case '-':
      memory[cell]--; break;
     case ',':
        memory[cell] = getc();
     case '.':
      if(memory[cell] < 32)
        printf("%i",memory[cell]);
      else
        putchar(memory[cell]);
      break;
    case '>':
      cell++; break;
    case '<':
      cell--; break;
    case '[':
        for (bracket_flag = 1; bracket_flag; c++)
          if (p[c] == '[')
            bracket_flag++;
          else if (p[c] == ']')
            bracket_flag--;
    break;
    case ']':
        c -= 2;  
        for (bracket_flag=1; bracket_flag; c--)
          if (p[c] == ']')
            bracket_flag++;
          else if (p[c] == '[')
            bracket_flag--;
        c++;
    break;
    case '$':
      bfput(memory[cell]);
    break;
    default:
      break;
    }
    c++;
  }
  fclose(bffile);
  puts("\n");
}

int main(int argc, char** argv)
{
  if(argc <= 1) 
   return 1;
  
  if(strcmp(argv[1], "-s") == 0)
   bfexec(argv[2]);
  return 0;
}
