#pragma once
#include "task.c"
#include "kb.c"
#include "fs.h"
#define __USER_DENIED  // user denied access to certain functions
#define __USER_ALLOWED
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

char* __USER_DENIED loadpwd() // xor decrypt the password
{
  struct buf* x = TALLOC(struct buf);
  _read(20, x, 128);
  int n;
  while(x->data[n])
    x->data[n] = x->data[n] ^ 0x1;
}



int setuid(int _id){
  if(_id > 0)
    cnuser.uid = _id;
  else{
    char* x = getl(); // gets the password from kbd
    char* p = loadpwd();
    if(strcmp(x,p) == 0)
      cnuser.uid = 0, return 0;
    kprint("\nerror: wrong password");
    free(x), free(p);
  }
}