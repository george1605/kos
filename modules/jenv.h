#pragma once
#include "../lib.c"
#include "../process.h"
#define __JAVA__ 0
#define JCONSOLE 1
#define JGUI 2
#define JWEB 4

struct jvm {
  struct proc jproc;
  int inst;
  int flags;
};

struct jvm jvm_init(){
  struct jvm u;
  u.jproc = prcreat("JVM");
  return u;
}