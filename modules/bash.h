#include "../lib.c"
#include "../mem.h"
#include "../fs.h"
#include "../user.h"
int _seplen = 0;

char* strsep(char* s, char* delm)
{
    static int currIndex = 0;
    if(!s || !delm || s[currIndex] == '\0')
      return (char*)0;

    char* W = (char*)alloc(0,100);
    int i = currIndex, k = 0, j = 0;

    while (s[i] != '\0'){
        j = 0;
        while (delm[j] != '\0'){
            if (s[i] != delm[j])
                W[k] = s[i];
            else goto It;
            j++;
        }

        i++;
        k++;
    }
It:
    W[i] = 0;
    currIndex = i+1;
    _seplen = i;
    return W;
}

void getargs(char* str, char* args[32]){
  int i = 0;
  char* p = strsep(str," |>");
  while(p != 0 && i < 32){
    args[i] = p;
    i++;
    p = strsep(str + _seplen," |>");
  }
}

void cmdexec(char* str){
  char* args[32];
  getargs(str,args);
  char* comm = args[0];
  if(strcmp(comm,"echo") == 0){
    kprint(args[1]);
  }else if(strcmp(comm,"mkdir")){
    if(str[6] == 0) return;
    mkdir(str + 6, &root);
  }else{
    sys_execv(args[0], args + 4);
  }
}

void argexec(int argc, char** argv){
  if(strcmp(argv[0], "sudo"))
  {
    kprintf("password: ");
    setuid(0);
    argexec(argc - 1, argv + sizeof(char*));
  }else if(strcmp(argv[0], "touch")){
    fcreat(argv[1]);
  }else if(strcmp(argv[0], "shutdown")){
    glsig = SHUTDOWN;
  }else{
    kprint("Unknown command\n");
  }
}