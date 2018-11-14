#ifndef __LIBC_H__
#define __LIBC_H__

#include <stdint.h>
#include <stddef.h>
#include <os.h>

// string.h
void *memset(void *b, int c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
size_t strlen(const char* s);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char * strcat(char * dst, const char * src);
char * strncat(char * front, const char * back, size_t count);

// stdlib.h
char *itoa(int n);
void srand(unsigned int seed);
int rand();

// stdio.h
int printf(const char* fmt, ...);
int sprintf(char* out, const char* format, ...);
int snprintf(char* s, size_t n, const char* format, ...);

// assert.h
#ifdef NDEBUG
  #define assert(ignore) ((void)0)
#else
  #define assert(cond) \
    do { \
      if (!(cond)) { \
        printf("Assertion fail at %s:%d\n", __FILE__, __LINE__); \
        _halt(1); \
      } \
    } while (0)
#endif

#endif
