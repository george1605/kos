#ifndef _ERROR_H
#define _ERROR_H

extern void __REDIRECT (__error_alias, (int __status, int __errnum,
                                        __const char *__format, ...),
                        error)
  __attribute__ ((__format__ (__printf__, 3, 4)));
extern void __REDIRECT (__error_noreturn, (int __status, int __errnum,
                                           __const char *__format, ...),
                        error)
  __attribute__ ((__noreturn__, __format__ (__printf__, 3, 4)));


/* If we know the function will never return make sure the compiler
   realizes that, too.  */
__extern_always_inline void
error (int __status, int __errnum, __const char *__format, ...)
{
  if (__builtin_constant_p (__status) && __status != 0)
    __error_noreturn (__status, __errnum, __format, __va_arg_pack ());
  else
    __error_alias (__status, __errnum, __format, __va_arg_pack ());
}


extern void __REDIRECT (__error_at_line_alias, (int __status, int __errnum,
                                                __const char *__fname,
                                                unsigned int __line,
                                                __const char *__format, ...),
                        error_at_line)
  __attribute__ ((__format__ (__printf__, 5, 6)));
extern void __REDIRECT (__error_at_line_noreturn, (int __status, int __errnum,
                                                   __const char *__fname,
                                                   unsigned int __line,
                                                   __const char *__format,
                                                   ...),
                        error_at_line)
  __attribute__ ((__noreturn__, __format__ (__printf__, 5, 6)));


/* If we know the function will never return make sure the compiler
   realizes that, too.  */
__extern_always_inline void
error_at_line (int __status, int __errnum, __const char *__fname,
               unsigned int __line,__const char *__format, ...)
{
  if (__builtin_constant_p (__status) && __status != 0)
    __error_at_line_noreturn (__status, __errnum, __fname, __line, __format,
                              __va_arg_pack ());
  else
    __error_at_line_alias (__status, __errnum, __fname, __line,
                           __format, __va_arg_pack ());
}

#endif