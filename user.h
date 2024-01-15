#pragma once
#include "task.c"
#include "kb.c"
#include "fs.h"
#define __USER_DENIED  // user denied access to certain functions
#define __USER_ALLOWED
#define MAIN_USER 0x10
#define NORM_USER 0x20
#define MAX_USERS 512 / sizeof(struct user)
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
  _read(20, x, 512ULL);
  int n = 0;
  while(x->data[n]) {
    x->data[n] = x->data[n] ^ 0x1;
    n++;
  }
}

int setuid(int _id){
  if(_id > 0)
    cnuser.uid = _id;
  else{
    char* x = getl(); // gets the password from kbd
    char* p = loadpwd();
    if(strcmp(x,p) == 0) {
      cnuser.uid = 0;
      return 0;
    }
    kprint("\nerror: wrong password");
    free(x), free(p);
    return 1;
  }
  return 0;
}

void user_init()
{
  int fd = prfopen(myproc(), "/home/users", F_READ);
  struct buf buf;
  _read(fd, &buf, 512ULL);
  if(((struct user*)buf.data)->uid == 0)
  {
    printf("Please provide password for root");
    if(setuid(0))
      return;    
  }
  memcpy(&cnuser, buf.data, sizeof(struct user));
}

void user_load(struct buf* buf, int num, struct user* user)
{
  char* buf = (char*)(buf->data + num * sizeof(struct user));
  memcpy(user, buf, sizeof(struct user));
}

void group_setup(struct group* group, int users)
{
  if(users > MAX_USERS)
    users = MAX_USERS;
  group->users = kalloc(sizeof(struct user) * users, KERN_MEM);
  group->n = 0;
}

void group_load(struct buf* buf, int gid, struct group* group)
{
  struct user us;
  for(int i = 0;i < MAX_USERS;i++)
  {
    user_load(buf, i, &us);
    if(us.gid == gid)
      group->users[group->n++] = us;   
  }
}

int user_exist_k(struct buf* buf, int uid)
{
  struct user us;
  for(int i = 0;i < MAX_USERS;i++)
  {
    user_load(buf, i, &us);
    if(us.uid == uid)
      return 1;   
  }
  return 0;
}

int user_setid(int uid)
{
  struct proc* pr = myproc();
  int fd = prfopen(pr, "/home/users", F_READ);
  struct buf buf;
  _read(fd, &buf, 512ULL);
  if(user_exist_k(&buf, uid))
  {
    return -1;
  }
  prfclose(pr, fd);
  return 0;
}

void user_fsread(char* name, char* buffer)
{
  if(cnuser.perm & F_READ) {
    fsread(name, buffer);
  }
}

int user_fsexist(char* fn)
{
  return fsexist(cnuser.cwd, fn);
}

void user_chdir(char* name)
{
  if(fs_dev->fs->exist(name, fs_dev, fs_dev->priv))
  {
    cnuser.cwd = name;
  }
}

void user_fsopen(char* name, int perms)
{
  char* path = kalloc(100, KERN_MEM);
  if((perms & F_WRITE) && (perms & F_EXEC)) // cannot be WX!
    perms &= ~F_WRITE;
  if(name[0] == '.' && name[1] == '/')
  {
    strcpy(path, cnuser.cwd);
    strcpy(path + strlen(cnuser.cwd), name + 2);
    free(path);
    return prfopen(myproc(), path, perms);
  }
  free(path);
  return prfopen(myproc(), name, perms);
}

void user_remove()
{

}

void user_ls(struct group* group, char* buffer, size_t sz)
{
  char* name;
  size_t uid;
  va_list list;
  if(group == NULL_PTR)
    return;
  for(int i = 0;i < group->n;i++)
  {
    name = group->users[i].name;
    uid = group->users[i].uid;
    va_start(list, name);
    vsnprintf(buffer, sz, "User: name = %s, uid = %i\n", list);
  }
  va_end(list);
}