/*
 * Taken from:
 * https://github.com/nervosnetwork/ckb-js-vm/blob/c8b2faed3886f4d7e0ca1b313bfad437c91ce94d/include/c-stdlib/src/malloc_impl.c
 */
#define CKB_MALLOC_DECLARATION_ONLY 1
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <memory.h>

#ifndef CKB_BRK_MIN
extern char _end[]; /* _end is set in the linker */
#define CKB_BRK_MIN ((uintptr_t)&_end)
#endif
#ifndef CKB_BRK_MAX
#define CKB_BRK_MAX 0x00300000
#endif

struct chunk {
    size_t psize, csize;
    struct chunk *next, *prev;
};

struct bin {
    volatile int lock[2];
    struct chunk *head;
    struct chunk *tail;
};

#define CKB_SIZE_ALIGN (4 * sizeof(size_t))
#define CKB_SIZE_MASK (-CKB_SIZE_ALIGN)
#define CKB_OVERHEAD (2 * sizeof(size_t))
#define CKB_DONTCARE 16
#define CKB_RECLAIM 163840
#define CKB_MMAP_THRESHOLD (0x1c00 * CKB_SIZE_ALIGN)

#define CKB_CHUNK_SIZE(c) ((c)->csize & -2)
#define CKB_CHUNK_PSIZE(c) ((c)->psize & -2)
#define CKB_PREV_CHUNK(c) ((struct chunk *)((char *)(c)-CKB_CHUNK_PSIZE(c)))
#define CKB_NEXT_CHUNK(c) ((struct chunk *)((char *)(c) + CKB_CHUNK_SIZE(c)))
#define CKB_MEM_TO_CHUNK(p) (struct chunk *)((char *)(p)-CKB_OVERHEAD)
#define CKB_CHUNK_TO_MEM(c) (void *)((char *)(c) + CKB_OVERHEAD)
#define CKB_BIN_TO_CHUNK(i) (CKB_MEM_TO_CHUNK(&mal.bins[i].head))
#define CKB_C_INUSE ((size_t)1)
#define CKB_IS_MMAPPED(c) !((c)->csize & (CKB_C_INUSE))
#define CKB_PAGE_SIZE 4096
void __bin_chunk(struct chunk *);
int ckb_exit(int8_t code);
static inline void a_crash() { ckb_exit(-1); }
void free(void *p);

static inline void a_and_64(volatile uint64_t *p, uint64_t v) { *p &= v; }

static inline void a_or_64(volatile uint64_t *p, uint64_t v) { *p |= v; }

static uintptr_t s_program_break = 0;
static uintptr_t s_brk_min = CKB_BRK_MIN;
static uintptr_t s_brk_max = CKB_BRK_MAX;

void malloc_config(uintptr_t min, uintptr_t max) {
    s_brk_min = min;
    s_brk_max = max;
    s_program_break = 0;
}

size_t malloc_usage() {
    size_t high = (size_t)s_program_break;
    size_t low = (size_t)s_brk_min;
    return high - low;
}

void *_sbrk(uintptr_t incr) {
    if (!s_program_break) {
        s_program_break = s_brk_min;
        s_program_break += -s_program_break & (CKB_PAGE_SIZE - 1);
    }
    if ((s_program_break + incr) > s_brk_max) {
        return (void *)-1;
    }

    uintptr_t start = s_program_break;
    s_program_break += incr;
    return (void *)start;
}

static struct {
    volatile uint64_t binmap;
    struct bin bins[64];
    volatile int split_merge_lock[2];
} mal;

static inline void lock_bin(int i) {
    if (!mal.bins[i].head) mal.bins[i].head = mal.bins[i].tail = CKB_BIN_TO_CHUNK(i);
}

static inline void unlock_bin(int i) {}

#if 0
static int first_set(uint64_t x) {
  // TODO: use RISC-V asm
  static const char debruijn64[64] = {
      0,  1,  2,  53, 3,  7,  54, 27, 4,  38, 41, 8,  34, 55, 48, 28,
      62, 5,  39, 46, 44, 42, 22, 9,  24, 35, 59, 56, 49, 18, 29, 11,
      63, 52, 6,  26, 37, 40, 33, 47, 61, 45, 43, 21, 23, 58, 17, 10,
      51, 25, 36, 32, 60, 20, 57, 16, 50, 31, 19, 15, 30, 14, 13, 12};
  static const char debruijn32[32] = {
      0,  1,  23, 2,  29, 24, 19, 3,  30, 27, 25, 11, 20, 8, 4,  13,
      31, 22, 28, 18, 26, 10, 7,  12, 21, 17, 9,  6,  16, 5, 15, 14};
  if (sizeof(long) < 8) {
    uint32_t y = x;
    if (!y) {
      y = x >> 32;
      return 32 + debruijn32[(y & -y) * 0x076be629 >> 27];
    }
    return debruijn32[(y & -y) * 0x076be629 >> 27];
  }
  return debruijn64[(x & -x) * 0x022fdd63cc95386dull >> 58];
}

#else

static int __attribute__((naked)) first_set(uint64_t x) {
    __asm__(".byte  0x13, 0x15, 0x15, 0x60");
    __asm__("ret");
}

#endif

static const unsigned char bin_tab[60] = {
    32, 33, 34, 35, 36, 36, 37, 37, 38, 38, 39, 39, 40, 40, 40, 40, 41, 41, 41, 41,
    42, 42, 42, 42, 43, 43, 43, 43, 44, 44, 44, 44, 44, 44, 44, 44, 45, 45, 45, 45,
    45, 45, 45, 45, 46, 46, 46, 46, 46, 46, 46, 46, 47, 47, 47, 47, 47, 47, 47, 47,
};

static int bin_index(size_t x) {
    x = x / CKB_SIZE_ALIGN - 1;
    if (x <= 32) return x;
    if (x < 512) return bin_tab[x / 8 - 4];
    if (x > 0x1c00) return 63;
    return bin_tab[x / 128 - 4] + 16;
}

static int bin_index_up(size_t x) {
    x = x / CKB_SIZE_ALIGN - 1;
    if (x <= 32) return x;
    x--;
    if (x < 512) return bin_tab[x / 8 - 4] + 1;
    return bin_tab[x / 128 - 4] + 17;
}

static void *__expand_heap(size_t *pn) {
    size_t n = *pn;
    n += -n & (CKB_PAGE_SIZE - 1);

    void *p = _sbrk(n);
    if (p == (void *)-1) {
        return 0;
    }
    *pn = n;
    return p;
}

static struct chunk *expand_heap(size_t n) {
    static void *end;
    void *p;
    struct chunk *w;

    /* The argument n already accounts for the caller's chunk
     * CKB_OVERHEAD needs, but if the heap can't be extended in-place,
     * we need room for an extra zero-sized sentinel chunk. */
    n += CKB_SIZE_ALIGN;

    p = __expand_heap(&n);
    if (!p) return 0;
    /* If not just expanding existing space, we need to make a
     * new sentinel chunk below the allocated space. */
    if (p != end) {
        /* Valid/safe because of the prologue increment. */
        n -= CKB_SIZE_ALIGN;
        p = (char *)p + CKB_SIZE_ALIGN;
        w = CKB_MEM_TO_CHUNK(p);
        w->psize = 0 | CKB_C_INUSE;
    }

    /* Record new heap end and fill in footer. */
    end = (char *)p + n;
    w = CKB_MEM_TO_CHUNK(end);
    w->psize = n | CKB_C_INUSE;
    w->csize = 0 | CKB_C_INUSE;

    /* Fill in header, which may be new or may be replacing a
     * zero-size sentinel header at the old end-of-heap. */
    w = CKB_MEM_TO_CHUNK(p);
    w->csize = n | CKB_C_INUSE;

    return w;
}

static int adjust_size(size_t *n) {
    /* Result of pointer difference must fit in ptrdiff_t. */
    if (*n - 1 > INT64_MAX - CKB_SIZE_ALIGN - CKB_PAGE_SIZE) {
        if (*n) {
            return -1;
        } else {
            *n = CKB_SIZE_ALIGN;
            return 0;
        }
    }
    *n = (*n + CKB_OVERHEAD + CKB_SIZE_ALIGN - 1) & CKB_SIZE_MASK;
    return 0;
}

static void unbin(struct chunk *c, int i) {
    if (c->prev == c->next) a_and_64(&mal.binmap, ~(1ULL << i));
    c->prev->next = c->next;
    c->next->prev = c->prev;
    c->csize |= CKB_C_INUSE;
    CKB_NEXT_CHUNK(c)->psize |= CKB_C_INUSE;
}

static void bin_chunk(struct chunk *self, int i) {
    self->next = CKB_BIN_TO_CHUNK(i);
    self->prev = mal.bins[i].tail;
    self->next->prev = self;
    self->prev->next = self;
    if (self->prev == CKB_BIN_TO_CHUNK(i)) a_or_64(&mal.binmap, 1ULL << i);
}

static void trim(struct chunk *self, size_t n) {
    size_t n1 = CKB_CHUNK_SIZE(self);
    struct chunk *next, *split;

    if (n >= n1 - CKB_DONTCARE) return;

    next = CKB_NEXT_CHUNK(self);
    split = (void *)((char *)self + n);

    split->psize = n | CKB_C_INUSE;
    split->csize = n1 - n;
    next->psize = n1 - n;
    self->csize = n | CKB_C_INUSE;

    int i = bin_index(n1 - n);
    lock_bin(i);

    bin_chunk(split, i);

    unlock_bin(i);
}

void *malloc(size_t n) {
    struct chunk *c;
    int i, j;
    uint64_t mask;

    if (adjust_size(&n) < 0) return 0;

    if (n >= CKB_MMAP_THRESHOLD) {
        // TODO: don't support too large memory
        return 0;
    }

    i = bin_index_up(n);
    if (i < 63 && (mal.binmap & (1ULL << i))) {
        lock_bin(i);
        c = mal.bins[i].head;
        if (c != CKB_BIN_TO_CHUNK(i) && CKB_CHUNK_SIZE(c) - n <= CKB_DONTCARE) {
            unbin(c, i);
            unlock_bin(i);
            return CKB_CHUNK_TO_MEM(c);
        }
        unlock_bin(i);
    }
    for (mask = mal.binmap & -(1ULL << i); mask; mask -= (mask & -mask)) {
        j = first_set(mask);
        lock_bin(j);
        c = mal.bins[j].head;
        if (c != CKB_BIN_TO_CHUNK(j)) {
            unbin(c, j);
            unlock_bin(j);
            break;
        }
        unlock_bin(j);
    }
    if (!mask) {
        c = expand_heap(n);
        if (!c) {
            return 0;
        }
    }
    trim(c, n);
    return CKB_CHUNK_TO_MEM(c);
}

void *realloc(void *p, size_t n) {
    struct chunk *self, *next;
    size_t n0;
    void *new;

    if (!p) return malloc(n);

    if (adjust_size(&n) < 0) return 0;

    self = CKB_MEM_TO_CHUNK(p);
    n0 = CKB_CHUNK_SIZE(self);

    if (n <= n0 && n0 - n <= CKB_DONTCARE) return p;

    next = CKB_NEXT_CHUNK(self);

    /* Crash on corrupted footer (likely from buffer overflow) */
    if (next->psize != self->csize) a_crash();

    if (n < n0) {
        int i = bin_index_up(n);
        int j = bin_index(n0);
        if (i < j && (mal.binmap & (1ULL << i))) goto copy_realloc;
        struct chunk *split = (void *)((char *)self + n);
        self->csize = split->psize = n | CKB_C_INUSE;
        split->csize = next->psize = (n0 - n) | CKB_C_INUSE;
        __bin_chunk(split);
        return CKB_CHUNK_TO_MEM(self);
    }

    size_t nsize = next->csize & CKB_C_INUSE ? 0 : CKB_CHUNK_SIZE(next);
    if (n0 + nsize >= n) {
        int i = bin_index(nsize);
        lock_bin(i);
        if (!(next->csize & CKB_C_INUSE)) {
            unbin(next, i);
            unlock_bin(i);
            next = CKB_NEXT_CHUNK(next);
            self->csize = next->psize = (n0 + nsize) | CKB_C_INUSE;
            trim(self, n);
            return CKB_CHUNK_TO_MEM(self);
        }
        unlock_bin(i);
    }
copy_realloc:
    /* As a last resort, allocate a new chunk and copy to it. */
    new = malloc(n - CKB_OVERHEAD);
    if (!new) return 0;
    memcpy(new, p, (n < n0 ? n : n0) - CKB_OVERHEAD);
    free(CKB_CHUNK_TO_MEM(self));
    return new;
}

void __bin_chunk(struct chunk *self) {
    struct chunk *next = CKB_NEXT_CHUNK(self);

    /* Crash on corrupted footer (likely from buffer overflow) */
    if (next->psize != self->csize) a_crash();

    size_t osize = CKB_CHUNK_SIZE(self), size = osize;

    /* Since we hold split_merge_lock, only transition from free to
     * in-use can race; in-use to free is impossible */
    size_t psize = self->psize & CKB_C_INUSE ? 0 : CKB_CHUNK_PSIZE(self);
    size_t nsize = next->csize & CKB_C_INUSE ? 0 : CKB_CHUNK_SIZE(next);

    if (psize) {
        int i = bin_index(psize);
        lock_bin(i);
        if (!(self->psize & CKB_C_INUSE)) {
            struct chunk *prev = CKB_PREV_CHUNK(self);
            unbin(prev, i);
            self = prev;
            size += psize;
        }
        unlock_bin(i);
    }
    if (nsize) {
        int i = bin_index(nsize);
        lock_bin(i);
        if (!(next->csize & CKB_C_INUSE)) {
            unbin(next, i);
            next = CKB_NEXT_CHUNK(next);
            size += nsize;
        }
        unlock_bin(i);
    }

    int i = bin_index(size);
    lock_bin(i);

    self->csize = size;
    next->psize = size;
    bin_chunk(self, i);

    unlock_bin(i);
}

void free(void *p) {
    if (!p) return;
    struct chunk *self = CKB_MEM_TO_CHUNK(p);
    __bin_chunk(self);
}

size_t malloc_usable_size(void *ptr) {
    struct chunk *c = CKB_MEM_TO_CHUNK(ptr);
    return CKB_CHUNK_PSIZE(c);
}
