#pragma once
#include "../process.h"

class Process {
private:
 struct proc _Process;
public:
 char* name;
 Process(){}
 ~Process(){
  
 }
 Process(const char* name){ 
  this->name = (char*)name;
  prinit(_Process,0);
 }
 void exec(int argc,char** argv){
   
 }
 void fork(){
   struct proc u = prcreat("NULL");
   u.parent = &(_Process);
 }
};
