#pragma once
#include "task.c"
#include "kb.c"
#define MAIN_USER 0x10
#define NORM_USER 0x20
int lastuid = 1;
int lastgid = 1;
char* nullargv[] = { (char*)0 };

struct user {
  char* name;
  char* cwd;
  int perm;
  int uid;
  int gid;
} cnuser;

struct group {
  char* name;
  struct user* users;
  int n;
  int gid;
};

void usinit(char* name){
  cnuser.name = name;
  cnuser.perm = MAIN_USER;
}

void usexec(struct task u){
 if(cnuser.perm >= u.perm)
  exectask(u);
} 

void usswitch(struct user t){
  cnuser = t;
}

void usadd(struct user i, struct group u){
    u.users[++u.n] = i;
}

void ussetcwd(char* _cwd){
  cnuser.cwd = _cwd;
}

int setuid(int _id){
  if(_id > 0)
    cnuser.uid = _id;
  else{
    char* x = gets(32); // gets the password from kbd
    if(strcmp(x,"***") == 0)
      cnuser.uid = 0, return 0;
    kprint("\nerror: wrong password");
    free(x);
  }
}