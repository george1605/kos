#pragma once
#define EFIAPI 
#define VOID void
#define MEGA 1024 * 1024
#define GIGA 1024 * MEGA
#define WBINVD() asm volatile("wbinvd");
#define EFI_SUCCESS 0
#define EFI_FAILURE 1
void (*BIOS32_HELPER)() = (void*)0x7C00;
typedef void* EFI_HANDLE;
typedef void* EFI_PHYSICAL_ADDRESS;
typedef unsigned long EFI_VIRTUAL_ADDRESS;
typedef unsigned long long UINT64; 
typedef unsigned int UINT32;
typedef unsigned short UINT16;
typedef short CHAR16;
typedef char CHAR8;

typedef int EFI_STATUS;

#define EFI_PAGE_SIZE   4 * 1024
#define EFI_PAGE_MASK   0xFFF
#define EFI_PAGE_SHIFT  12

#define EFI_RESET_COLD 0
#define EFI_RESET_WARM 1
#define EFI_RESET_SHUTDOWN 2

typedef struct _EFI_INPUT_KEY {
 UINT16 ScanCode;
 CHAR16 UnicodeChar;
} EFI_INPUT_KEY;

void ReadKeyStroke(EFI_INPUT_KEY* InputKey){
   
}

EFI_STATUS CreateEvent(int Type,int Tpl){
  return 0;
}

typedef struct {
    UINT32                Type;           
    UINT32                Pad;
    EFI_PHYSICAL_ADDRESS  PhysicalStart;  
    EFI_VIRTUAL_ADDRESS   VirtualStart;   
    UINT64                NumberOfPages;  
    UINT64                Attribute;      
} EFI_MEMORY_DESCRIPTOR;

void* AllocatePool(){
  return (void*)1;
}

void FreePool(void* mem){
  char* ptr = mem;
  UINT32 i = 0;
  while(*(ptr + i) != 0xdeadbeef){
    *(ptr + i) = 0;
    i++;
  }
}

void SetMem(void* mem, int size, int value){
  int i = 0;
  char* m = (char*)mem;
  while(m[i] != 0 && i < size){
    m[i] = value;
    i++;
  }
}

void Print(const char* u){

}

int bios_main(){
  Print("UEFI!!!");
  return 0;
}
