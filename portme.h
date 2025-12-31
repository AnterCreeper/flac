#ifndef __PORTME_H__
#define __PORTME_H__

#define HAVE_STDIO
#include <stdio.h>
#include <stdlib.h>

#define __BUILTIN_CLZ32     __builtin_clz

typedef int int32_t;

void __assert(const char* str, int line);
#define assert(x) __assert(x, __LINE__)

//#define STATIC_MEM    //comment out if we have malloc(size_t);

void portme_fread(void* dest, size_t len);
void portme_stream(int32_t left, int32_t right, int samplebits);

#endif
