#pragma once 
/* Support for MS Windows Functions
   Kernel Functions, Processes/Threads and Graphics(GUIs)
   Warning: The value of the macros may not correspond to the actual value
*/
#include "../port.h"
#include "../fs.h"
#include "../lib.c"
#include "../mutex.h"
#include "../gui.c"
#define IN
#define OUT
#define OPTIONAL
#define WINAPI
#ifndef __WINC__
 #define __WINC__ 2
#endif
#define STD_API(a) a
typedef int COLORREF;
#define RGB(_r,_g,_b) (COLORREF)((_r) | (_g << 8) | (_b << 16))

typedef char BYTE;
typedef short WORD;
typedef int DWORD;
typedef int INTPTR;
typedef int BOOLEAN;
typedef size_t SIZE_T;
typedef size_t UINT;
typedef unsigned short USHORT;
typedef wchar_t* PCHAR;
typedef void(*LPTHREAD_START_ROUTINE)(int argc,char** argv);
typedef const char* LPCSTR;
typedef const wchar_t* LPCWSTR;
typedef char* LPSTR;
typedef wchar_t* LPWSTR;
typedef long NTSTATUS;
typedef NTSTATUS KSTATUS;

typedef void* PVOID;
typedef void* HANDLE;
typedef void* HWND;
typedef void* HDC;
typedef void* HICON;
typedef void* HCURSOR;
typedef void* HINSTANCE;
typedef HANDLE* PHANDLE; // just a double-pointer
extern int errno;

#define MAKEINTRESOURCEA(i) ((LPSTR)((WORD)(i)))
#define MAKEINTRESOURCEW(i) ((LPWSTR)((WORD)(i)))
#define INVALID_HANDLE_VALUE -1
#define STATUS_SUCCESS                ((NTSTATUS)0x00000000L)
#define STATUS_BUFFER_OVERFLOW        ((NTSTATUS)0x80000005L)
#define STATUS_UNSUCCESSFUL           ((NTSTATUS)0xC0000001L)
#define STATUS_NOT_IMPLEMENTED        ((NTSTATUS)0xC0000002L)
#define STATUS_INFO_LENGTH_MISMATCH   ((NTSTATUS)0xC0000004L)

#define CURRENT_PROCESS ((HANDLE) -1)
#define CURRENT_THREAD  ((HANDLE) -2)
#define NtCurrentProcess CURRENT_PROCESS

typedef struct tagWindow {
  void(*loop)();

} Window;

HWND AllocWindow(){
  HWND u = (HWND)TALLOC(HWND);
  return u;
}

void FreeHandle(HANDLE u){
  free(u);
}

typedef struct _STRING {
  USHORT Length;
  USHORT MaximumLength;
  PCHAR  Buffer;
} STRING;

typedef STRING *PSTRING;

typedef struct _HBRUSH {
  SIZE_T unused;
} HBRUSH;

typedef struct {
  SIZE_T cbSize;
  SIZE_T style;
  int cbClsExtra;
  int cbWndExtra;
  HINSTANCE hInstance;
  HICON hIcon;
} WNDCLASSEX;

typedef enum {
   
} TIMER_TYPE;

NTSTATUS NtCreateTimer(
  OUT PHANDLE TimerHandle,
  IN  ACCESS_MASK DesiredAccess,
  IN  POBJECT_ATTRIBUTES OPTIONAL,
  IN  TIMER_TYPE TimerType
)
{
  size_t* t = alloc(0,4);
  TimerHandle[0] = (void*)t;
  return 0;
}

void NtCancelTimer(PHANDLE TimerHandle, int* CurrentState)
{

  free(TimerHandle[0]); //frees the memory
}

HBRUSH CreateSolidBrush(COLORREF lpColor)
{
   HBRUSH u;
   u.unused = (SIZE_T)lpColor;
   return u;
}

HANDLE NtCreateFile()
{
    return (HANDLE)0;

}

void DbgPrint(const char* string)
{
    kprint(string);
}

HANDLE CreateThread(SIZE_T StackSize,LPTHREAD_START_ROUTINE lpStartAddress)
{
  struct thread mythread = thcreat(0,StackSize);
  mythread.f = lpStartAddress;
  return (HANDLE)(&mythread);
}

HANDLE CreateProcess()
{
  struct proc u = prcreat("");
  return (HANDLE)(&u);
}

void ExitThread(DWORD swExitCode)
{
  cthread.state = KILLED;
}

DWORD GetCurrentProcessId()
{
  return (DWORD)tproc.pid;
}

HWND CreateWindow()
{
  return (DWORD)0;
}

void MessageBoxA(HWND Handle,LPCSTR lpText,LPCSTR lpCaption,UINT lpFlags)
{
  
}

NTSTATUS NtDisplayString(STRING String){
  kprint((char*)String.Buffer);
  return 0;
}

NTSTATUS NtCreateEvent(PHANDLE EventHandle, ACCESS_MASK DesiredAccess,
  POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,EVENT_TYPE EventType,
  BOOLEAN InitialState)
{

}

HWND GetConsoleWindow(void)
{

}

DWORD GetLastError()
{
  return (DWORD)errno;
}