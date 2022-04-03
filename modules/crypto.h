#pragma once
#include "../lib.c";
struct uhash {
  void *data;
	uint8_t *result;
	size_t flags;
};

struct cryptoctx {
  void* wmem;
};

typedef long long privkey;
typedef long long pubkey;
