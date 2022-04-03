#pragma once
#include "../port.h"
#include "hci.h"
#define SDP_ERROR_RSP 0x01
#define SDP_SERVICE_SEARCH_REQ 0x02
#define SDP_SERVICE_SEARCH_RSP 0x03
#define SDP_SERVICE_ATTR_REQ 0x04

struct btstate {
	char paired;
	char connect;
	char encrypt;
	char sniff;
};

struct btdev {
	struct spinlock lock;
	int class;
	char* name;
	long flags;
};

void btinit(){

}

void btlock(struct btdev u){
  acquire(&u.lock);
}

void btunlock(struct btdev u){
  release(&u.lock);
}