
#ifndef _ERROR_H
#define _ERROR_H 1

#include "features.h"


__BEGIN_DECLS

/* Print a message with `fprintf (stderr, FORMAT, ...)';
   if ERRNUM is nonzero, follow it with ": " and strerror (ERRNUM).
   If STATUS is nonzero, terminate the program with `exit (STATUS)'.  */

extern void error (int __status, int __errnum, __const char *__format, ...)
     __attribute__ ((__format__ (__printf__, 3, 4)));

extern void error_at_line (int __status, int __errnum, __const char *__fname,
                           unsigned int __lineno, __const char *__format, ...)
     __attribute__ ((__format__ (__printf__, 5, 6)));

/* If NULL, error will flush stdout, then print on stderr the program
   name, a colon and a space.  Otherwise, error will call this
   function without parameters instead.  */
extern void (*error_print_progname) (void);

/* This variable is incremented each time `error' is called.  */
extern unsigned int error_message_count;

/* Sometimes we want to have at most one error per line.  This
   variable controls whether this mode is selected or not.  */
extern int error_one_per_line;


#if defined __extern_always_inline && defined __va_arg_pack
# include "bits/error.h"
#endif

__END_DECLS

#endif /* error.h */