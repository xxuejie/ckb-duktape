#ifndef AUGMENTED_LIBC_STDARG_H_
#define AUGMENTED_LIBC_STDARG_H_

#include_next "stdarg.h"

#define va_copy(d, s) __builtin_va_copy(d, s)

#endif /* AUGMENTED_LIBC_STDARG_H_ */
