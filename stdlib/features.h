#ifndef _LOOSE_KERNEL_NAMES
# define __KERNEL_STRICT_NAMES
#endif

/* Always use ISO C things.  */
#define        __USE_ANSI        1

/* Convenience macros to test the versions of glibc and gcc.
   Use them like this:
   #if __GNUC_PREREQ (2,8)
   ... code requiring gcc 2.8 or later ...
   #endif
   Note - they won't work for gcc1 or glibc1, since the _MINOR macros
   were not defined then.  */
#if defined __GNUC__ && defined __GNUC_MINOR__
# define __GNUC_PREREQ(maj, min) \
        ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
# define __GNUC_PREREQ(maj, min) 0
#endif


/* If _BSD_SOURCE was defined by the user, favor BSD over POSIX.  */
#if defined _BSD_SOURCE && \
    !(defined _POSIX_SOURCE || defined _POSIX_C_SOURCE || \
      defined _XOPEN_SOURCE || defined _XOPEN_SOURCE_EXTENDED || \
      defined _GNU_SOURCE || defined _SVID_SOURCE)
# define __FAVOR_BSD        1
#endif

/* If _GNU_SOURCE was defined by the user, turn on all the other features.  */
#ifdef _GNU_SOURCE
# undef  _ISOC99_SOURCE
# define _ISOC99_SOURCE        1
# undef  _POSIX_SOURCE
# define _POSIX_SOURCE        1
# undef  _POSIX_C_SOURCE
# define _POSIX_C_SOURCE        200809L
# undef  _XOPEN_SOURCE
# define _XOPEN_SOURCE        700
# undef  _XOPEN_SOURCE_EXTENDED
# define _XOPEN_SOURCE_EXTENDED        1
# undef         _LARGEFILE64_SOURCE
# define _LARGEFILE64_SOURCE        1
# undef  _BSD_SOURCE
# define _BSD_SOURCE        1
# undef  _SVID_SOURCE
# define _SVID_SOURCE        1
# undef  _ATFILE_SOURCE
# define _ATFILE_SOURCE        1
#endif

/* If nothing (other than _GNU_SOURCE) is defined,
   define _BSD_SOURCE and _SVID_SOURCE.  */
#if (!defined __STRICT_ANSI__ && !defined _ISOC99_SOURCE && \
     !defined _POSIX_SOURCE && !defined _POSIX_C_SOURCE && \
     !defined _XOPEN_SOURCE && !defined _XOPEN_SOURCE_EXTENDED && \
     !defined _BSD_SOURCE && !defined _SVID_SOURCE)
# define _BSD_SOURCE        1
# define _SVID_SOURCE        1
#endif

/* This is to enable the ISO C99 extension.  Also recognize the old macro
   which was used prior to the standard acceptance.  This macro will
   eventually go away and the features enabled by default once the ISO C99
   standard is widely adopted.  */
#if (defined _ISOC99_SOURCE || defined _ISOC9X_SOURCE \
     || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L))
# define __USE_ISOC99        1
#endif

/* This is to enable the ISO C90 Amendment 1:1995 extension.  */
#if (defined _ISOC99_SOURCE || defined _ISOC9X_SOURCE \
     || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199409L))
# define __USE_ISOC95        1
#endif

/* If none of the ANSI/POSIX macros are defined, use POSIX.1 and POSIX.2
   (and IEEE Std 1003.1b-1993 unless _XOPEN_SOURCE is defined).  */
#if ((!defined __STRICT_ANSI__ || (_XOPEN_SOURCE - 0) >= 500) && \
     !defined _POSIX_SOURCE && !defined _POSIX_C_SOURCE)
# define _POSIX_SOURCE        1
# if defined _XOPEN_SOURCE && (_XOPEN_SOURCE - 0) < 500
#  define _POSIX_C_SOURCE        2
# elif defined _XOPEN_SOURCE && (_XOPEN_SOURCE - 0) < 600
#  define _POSIX_C_SOURCE        199506L
# elif defined _XOPEN_SOURCE && (_XOPEN_SOURCE - 0) < 700
#  define _POSIX_C_SOURCE        200112L
# else
#  define _POSIX_C_SOURCE        200809L
# endif
# define __USE_POSIX_IMPLICITLY        1
#endif

#if defined _POSIX_SOURCE || _POSIX_C_SOURCE >= 1 || defined _XOPEN_SOURCE
# define __USE_POSIX        1
#endif

#if defined _POSIX_C_SOURCE && _POSIX_C_SOURCE >= 2 || defined _XOPEN_SOURCE
# define __USE_POSIX2        1
#endif

#if (_POSIX_C_SOURCE - 0) >= 199309L
# define __USE_POSIX199309        1
#endif

#if (_POSIX_C_SOURCE - 0) >= 199506L
# define __USE_POSIX199506        1
#endif

#if (_POSIX_C_SOURCE - 0) >= 200112L
# define __USE_XOPEN2K                1
# undef __USE_ISOC99
# define __USE_ISOC99                1
#endif

#if (_POSIX_C_SOURCE - 0) >= 200809L
# define __USE_XOPEN2K8                1
# undef  _ATFILE_SOURCE
# define _ATFILE_SOURCE        1
#endif

#ifdef        _XOPEN_SOURCE
# define __USE_XOPEN        1
# if (_XOPEN_SOURCE - 0) >= 500
#  define __USE_XOPEN_EXTENDED        1
#  define __USE_UNIX98        1
#  undef _LARGEFILE_SOURCE
#  define _LARGEFILE_SOURCE        1
#  if (_XOPEN_SOURCE - 0) >= 600
#   if (_XOPEN_SOURCE - 0) >= 700
#    define __USE_XOPEN2K8        1
#   endif
#   define __USE_XOPEN2K        1
#   undef __USE_ISOC99
#   define __USE_ISOC99                1
#  endif
# else
#  ifdef _XOPEN_SOURCE_EXTENDED
#   define __USE_XOPEN_EXTENDED        1
#  endif
# endif
#endif

#ifdef _LARGEFILE_SOURCE
# define __USE_LARGEFILE        1
#endif

#ifdef _LARGEFILE64_SOURCE
# define __USE_LARGEFILE64        1
#endif

#if defined _FILE_OFFSET_BITS && _FILE_OFFSET_BITS == 64
# define __USE_FILE_OFFSET64        1
#endif

#if defined _BSD_SOURCE || defined _SVID_SOURCE
# define __USE_MISC        1
#endif

#ifdef        _BSD_SOURCE
# define __USE_BSD        1
#endif

#ifdef        _SVID_SOURCE
# define __USE_SVID        1
#endif

#ifdef        _ATFILE_SOURCE
# define __USE_ATFILE        1
#endif

#ifdef        _GNU_SOURCE
# define __USE_GNU        1
#endif

#if defined _REENTRANT || defined _THREAD_SAFE
# define __USE_REENTRANT        1
#endif

#if defined _FORTIFY_SOURCE && _FORTIFY_SOURCE > 0 \
    && __GNUC_PREREQ (4, 1) && defined __OPTIMIZE__ && __OPTIMIZE__ > 0
# if _FORTIFY_SOURCE > 1
#  define __USE_FORTIFY_LEVEL 2
# else
#  define __USE_FORTIFY_LEVEL 1
# endif
#else
# define __USE_FORTIFY_LEVEL 0
#endif

/* Define __STDC_IEC_559__ and other similar macros.  */
#include <bits/predefs.h>

/* wchar_t uses ISO 10646-1 (2nd ed., published 2000-09-15) / Unicode 3.1.  */
#define __STDC_ISO_10646__                200009L

/* This macro indicates that the installed library is the GNU C Library.
   For historic reasons the value now is 6 and this will stay from now
   on.  The use of this variable is deprecated.  Use __GLIBC__ and
   __GLIBC_MINOR__ now (see below) when you want to test for a specific
   GNU C library version and use the values in <gnu/lib-names.h> to get
   the sonames of the shared libraries.  */
#undef  __GNU_LIBRARY__
#define __GNU_LIBRARY__ 6

/* Major and minor version number of the GNU C library package.  Use
   these macros to test for features in specific releases.  */
#define        __GLIBC__        2
#define        __GLIBC_MINOR__        11

#define __GLIBC_PREREQ(maj, min) \
        ((__GLIBC__ << 16) + __GLIBC_MINOR__ >= ((maj) << 16) + (min))

/* Decide whether a compiler supports the long long datatypes.  */
#if defined __GNUC__ \
    || (defined __PGI && defined __i386__ ) \
    || (defined __INTEL_COMPILER && (defined __i386__ || defined __ia64__)) \
    || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L)
# define __GLIBC_HAVE_LONG_LONG        1
#endif

/* This is here only because every header file already includes this one.  */
#ifndef __ASSEMBLER__
# ifndef _SYS_CDEFS_H
#  include <sys/cdefs.h>
# endif

/* If we don't have __REDIRECT, prototypes will be missing if
   __USE_FILE_OFFSET64 but not __USE_LARGEFILE[64]. */
# if defined __USE_FILE_OFFSET64 && !defined __REDIRECT
#  define __USE_LARGEFILE        1
#  define __USE_LARGEFILE64        1
# endif

#endif        /* !ASSEMBLER */

/* Decide whether we can define 'extern inline' functions in headers.  */
#if __GNUC_PREREQ (2, 7) && defined __OPTIMIZE__ \
    && !defined __OPTIMIZE_SIZE__ && !defined __NO_INLINE__ \
    && defined __extern_inline
# define __USE_EXTERN_INLINES        1
#endif

/* There are some functions that must be declared 'extern inline' even with
   -Os when building LIBC, or they'll end up undefined.  */
#if __GNUC_PREREQ (2, 7) && defined __OPTIMIZE__ \
    && (defined _LIBC || !defined __OPTIMIZE_SIZE__) && !defined __NO_INLINE__ \
    && defined __extern_inline
# define __USE_EXTERN_INLINES_IN_LIBC        1
#endif


/* This is here only because every header file already includes this one.
   Get the definitions of all the appropriate `__stub_FUNCTION' symbols.
   <gnu/stubs.h> contains `#define __stub_FUNCTION' when FUNCTION is a stub
   that will always return failure (and set errno to ENOSYS).  */
#include <gnu/stubs.h>


#endif        /* features.h  */