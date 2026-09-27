#ifndef __PORTME_H__
#define __PORTME_H__

/**
 * fixed-width types that stay valid when int is 16-bit.
 * short is 16-bit and long is (at least) 32-bit on both
 * hosted 32/64-bit and bare-metal 16-bit targets.
 */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) && !defined(PORTME_NO_STDINT)
#include <stdint.h>
#else
typedef short          int16_t;
typedef unsigned short uint16_t;
typedef long           int32_t;
typedef unsigned long  uint32_t;
#endif

#ifdef HAVE_STDIO
#include <stdio.h>
#include <stdlib.h>
#endif

#include <limits.h>

/* count leading zeros of a 32-bit value, portable across int widths */
#define __BUILTIN_CLZ32(x)  (__builtin_clz((unsigned int)(x)) - (sizeof(unsigned int) * CHAR_BIT - 32))

void __assert(const char* str, int line);
#define assert(x) __assert(x, __LINE__)

//#define STATIC_MEM    //comment out if we have malloc(size_t);

/* read up to len bytes, return the number of bytes actually read (< len at end of stream) */
unsigned int portme_fread(void* dest, unsigned int len);
void portme_stream(int16_t left, int16_t right, int samplebits);

#endif
