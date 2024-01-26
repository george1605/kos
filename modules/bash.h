#include "../lib.c"
#include "../mem.h"
#include "../fs.h"
#include "../user.h"
#include "../elf.h"
#include "../console.h"
int _seplen = 0;

struct shell
{
  int argc;
  char** argv;
  struct console cons;
};

// in the binary dir (will make one that looks through PATH env later)
char* find_ex(char* arg0)
{
  char* buf = kalloc(300, KERN_MEM);
  memcpy(buf, "/dev/bin/", 9);
  memcpy(buf + 9, arg0, strlen(arg0));
  return buf;
}

void exec_shell(struct shell sh)
{
  char* path = find_ex(sh.argv[0]);
}

char* strsep(char* s, char* delm)
{
    static int currIndex = 0;
    if(!s || !delm || s[currIndex] == '\0')
      return (char*)NULL_PTR;

    char* W = (char*)kalloc(100, KERN_MEM);
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
    elfexec_ext(argv[0]);
    kprint("Unknown command\n");
  }
}

void at_exit()
{
  kdettach_console(myproc());
}

void shell_main(int argc, char** argv)
{
  struct console* cns = alloc_console();
  struct proc* pr = myproc();
  kclear_console(pr);
  addexit(pr, at_exit);
  klog(cns, "Bash Shell\n");
  klog(cns, "$ ");
  int exit = 0; // if exited
  char* buffer;
  while(!exit)
  {
    buffer = getl();
    argexec(argc, argv);
  }
  free(buffer); // we need to free the buffer
  kprexit(pr, 0);
}

void shell_as_start()
{
  kprint("Setting up the shell...\n");
  struct proc* p = prnew_k("shell", 64 * 1024);
  schedinit(p); 
}