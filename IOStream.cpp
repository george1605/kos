#pragma once
#include "vfs.h"

class IOStream {
private:
  int sid;
public:
  static void cwrite(const char* str){
    #if defined(VISUAL_OS) // graphics available
      kprint((char*)str);
    #endif
    #if !defined(VISUAL_OS)

    #endif
  }
}