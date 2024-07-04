#ifndef AUGMENTED_LIBC_INTTYPES_H_
#define AUGMENTED_LIBC_INTTYPES_H_

#include <stdint.h>

typedef int8_t int_fast8_t;
typedef int64_t int_fast16_t;
typedef int64_t int_fast32_t;
typedef int64_t int_fast64_t;

typedef int8_t  int_least8_t;
typedef int16_t int_least16_t;
typedef int32_t int_least32_t;
typedef int64_t int_least64_t;

typedef uint8_t uint_fast8_t;
typedef uint64_t uint_fast16_t;
typedef uint64_t uint_fast32_t;
typedef uint64_t uint_fast64_t;

typedef uint8_t  uint_least8_t;
typedef uint16_t uint_least16_t;
typedef uint32_t uint_least32_t;
typedef uint64_t uint_least64_t;

typedef int64_t intmax_t;
typedef uint64_t uintmax_t;

typedef int64_t ptrdiff_t;

#endif /* AUGMENTED_LIBC_INTTYPES_H_ */
