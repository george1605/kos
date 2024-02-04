#include "../features.h"
#include "../gnu/stubs.h"

/* Convenience types.  */
typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;

/* Fixed-size types, underlying types depend on word size and compiler.  */
typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;
#if __WORDSIZE == 64
typedef signed long int __int64_t;
typedef unsigned long int __uint64_t;
#elif defined __GLIBC_HAVE_LONG_LONG
__extension__ typedef signed long long int __int64_t;
__extension__ typedef unsigned long long int __uint64_t;
#endif

/* quad_t is also 64 bits.  */
#if __WORDSIZE == 64
typedef long int __quad_t;
typedef unsigned long int __u_quad_t;
#elif defined __GLIBC_HAVE_LONG_LONG
__extension__ typedef long long int __quad_t;
__extension__ typedef unsigned long long int __u_quad_t;
#else
typedef struct
{
  long __val[2];
} __quad_t;
typedef struct
{
  __u_long __val[2];
} __u_quad_t;
#endif

#define        __S16_TYPE                short int
#define __U16_TYPE                unsigned short int
#define        __S32_TYPE                int
#define __U32_TYPE                unsigned int
#define __SLONGWORD_TYPE        long int
#define __ULONGWORD_TYPE        unsigned long int
#if __WORDSIZE == 32
# define __SQUAD_TYPE                __quad_t
# define __UQUAD_TYPE                __u_quad_t
# define __SWORD_TYPE                int
# define __UWORD_TYPE                unsigned int
# define __SLONG32_TYPE                long int
# define __ULONG32_TYPE                unsigned long int
# define __S64_TYPE                __quad_t
# define __U64_TYPE                __u_quad_t
/* We want __extension__ before typedef's that use nonstandard base types
   such as `long long' in C89 mode.  */
# define __STD_TYPE                __extension__ typedef
#elif __WORDSIZE == 64
# define __SQUAD_TYPE                long int
# define __UQUAD_TYPE                unsigned long int
# define __SWORD_TYPE                long int
# define __UWORD_TYPE                unsigned long int
# define __SLONG32_TYPE                int
# define __ULONG32_TYPE                unsigned int
# define __S64_TYPE                long int
# define __U64_TYPE                unsigned long int
/* No need to mark the typedef with __extension__.   */
# define __STD_TYPE                typedef
#else
# error
#endif
#include "typesizes.h"        /* Defines __*_T_TYPE macros.  */

__STD_TYPE __DEV_T_TYPE __dev_t;        /* Type of device numbers.  */
__STD_TYPE __UID_T_TYPE __uid_t;        /* Type of user identifications.  */
__STD_TYPE __GID_T_TYPE __gid_t;        /* Type of group identifications.  */
__STD_TYPE __INO_T_TYPE __ino_t;        /* Type of file serial numbers.  */
__STD_TYPE __INO64_T_TYPE __ino64_t;        /* Type of file serial numbers (LFS).*/
__STD_TYPE __MODE_T_TYPE __mode_t;        /* Type of file attribute bitmasks.  */
__STD_TYPE __NLINK_T_TYPE __nlink_t;        /* Type of file link counts.  */
__STD_TYPE __OFF_T_TYPE __off_t;        /* Type of file sizes and offsets.  */
__STD_TYPE __OFF64_T_TYPE __off64_t;        /* Type of file sizes and offsets (LFS).  */
__STD_TYPE __PID_T_TYPE __pid_t;        /* Type of process identifications.  */
__STD_TYPE __FSID_T_TYPE __fsid_t;        /* Type of file system IDs.  */
__STD_TYPE __CLOCK_T_TYPE __clock_t;        /* Type of CPU usage counts.  */
__STD_TYPE __RLIM_T_TYPE __rlim_t;        /* Type for resource measurement.  */
__STD_TYPE __RLIM64_T_TYPE __rlim64_t;        /* Type for resource measurement (LFS).  */
__STD_TYPE __ID_T_TYPE __id_t;                /* General type for IDs.  */
__STD_TYPE __TIME_T_TYPE __time_t;        /* Seconds since the Epoch.  */
__STD_TYPE __USECONDS_T_TYPE __useconds_t; /* Count of microseconds.  */
__STD_TYPE __SUSECONDS_T_TYPE __suseconds_t; /* Signed count of microseconds.  */

__STD_TYPE __DADDR_T_TYPE __daddr_t;        /* The type of a disk address.  */
__STD_TYPE __SWBLK_T_TYPE __swblk_t;        /* Type of a swap block maybe?  */
__STD_TYPE __KEY_T_TYPE __key_t;        /* Type of an IPC key.  */

/* Clock ID used in clock and timer functions.  */
__STD_TYPE __CLOCKID_T_TYPE __clockid_t;

/* Timer ID returned by `timer_create'.  */
__STD_TYPE __TIMER_T_TYPE __timer_t;

/* Type to represent block size.  */
__STD_TYPE __BLKSIZE_T_TYPE __blksize_t;

/* Types from the Large File Support interface.  */

/* Type to count number of disk blocks.  */
__STD_TYPE __BLKCNT_T_TYPE __blkcnt_t;
__STD_TYPE __BLKCNT64_T_TYPE __blkcnt64_t;

/* Type to count file system blocks.  */
__STD_TYPE __FSBLKCNT_T_TYPE __fsblkcnt_t;
__STD_TYPE __FSBLKCNT64_T_TYPE __fsblkcnt64_t;

/* Type to count file system nodes.  */
__STD_TYPE __FSFILCNT_T_TYPE __fsfilcnt_t;
__STD_TYPE __FSFILCNT64_T_TYPE __fsfilcnt64_t;

__STD_TYPE __SSIZE_T_TYPE __ssize_t; /* Type of a byte count, or error.  */

/* These few don't really vary by system, they always correspond
   to one of the other defined types.  */
typedef __off64_t __loff_t;        /* Type of file sizes and offsets (LFS).  */
typedef __quad_t *__qaddr_t;
typedef char *__caddr_t;

/* Duplicates info from stdint.h but this is used in unistd.h.  */
__STD_TYPE __SWORD_TYPE __intptr_t;

/* Duplicate info from sys/socket.h.  */
__STD_TYPE __U32_TYPE __socklen_t;


#undef __STD_TYPE

#endif /* bits/types.h */