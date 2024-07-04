#ifndef AUGMENTED_LIBC_STDIO_H_
#define AUGMENTED_LIBC_STDIO_H_

#include_next "stdio.h"
#include <stdarg.h>
#include <stdlib.h>

#define snprintf snprintf_
#define sprintf sprintf_
#define vsnprintf vsnprintf_

int sprintf_(char *buffer, const char *format, ...);
int snprintf_(char *buffer, size_t count, const char *format, ...);
int vsnprintf_(char *buffer, size_t count, const char *format, va_list va);

#ifndef CKB_C_STDLIB_PRINTF
int sprintf_(char *buffer, const char *format, ...) {
  (void) buffer;
  (void) format;
  return 0;
}
int snprintf_(char *buffer, size_t count, const char *format, ...) {
  (void) buffer;
  (void) count;
  (void) format;
  return 0;
}
int vsnprintf_(char *buffer, size_t count, const char *format, va_list va) {
  (void) buffer;
  (void) count;
  (void) format;
  (void) va;
  return 0;
}
#endif

#endif /* AUGMENTED_LIBC_STDIO_H_ */
