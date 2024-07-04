#ifndef AUGMENTED_LIBC_SETJMP_H_
#define AUGMENTED_LIBC_SETJMP_H_

#include <entry.h>

#include "internal/types.h"

/*
 * Our setjmp implementation is actually a combination of:
 * https://android.googlesource.com/platform/bionic.git/+/eclair-release/libc/include/setjmp.h
 * and
 * https://git.musl-libc.org/cgit/musl/tree/include/setjmp.h?id=ab31e9d6a0fa7c5c408856c89df2dfb12c344039
 *
 * We are taking the simpler bits of the 2 to suit our needs.
 */
typedef long jmp_buf[14];

int setjmp (jmp_buf);
void longjmp (jmp_buf, int);

#endif /* AUGMENTED_LIBC_SETJMP_H_ */
