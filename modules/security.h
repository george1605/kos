/*
 * Security Update, took the model from Mobile Systems
*/

#pragma once
#include "net.h"
#include "system.h"
#include "user.h"
typedef void (*constraint_handler_t)(const char *message, void *ptr, size_t error);

char* sys_last_hash = "0f0f0f";

void security_find_hash(char** hash)
{
    // TO DO!
}

int security_check_hash(char* hash)
{
    char* p;
    security_find_hash(&p);
    return !strcmp(sys_last_hash, p);
}

void security_cleanup()
{
    
}

void security_lockdown()
{
    printf("Entered security lockdown.");
    unmount("/dev/net"); // unmaps the network
    security_cleanup();
    sys_fabort(); // abort
}