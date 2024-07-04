#ifndef AUGMENTED_LIBC_STDLIB_H_
#define AUGMENTED_LIBC_STDLIB_H_

#include_next "stdlib.h"

extern long strtol(const char *, char **, int);
__attribute__((noreturn)) void abort (void);

#endif /* AUGMENTED_LIBC_STDLIB_H_ */
