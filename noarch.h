#define __DIFF(a, b) (int)(&b - &a)

struct refptr
{
  void* ptr;
  int count;
};
