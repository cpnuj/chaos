#include "string.h"

#include "debug.h"
#include "stdnull.h"

void memset(void* dst_, uint8_t value, uint32_t size) {
  ASSERT(dst_ != NULL);
  uint8_t* dst = (uint8_t*)dst_;
  while (size-- > 0) *dst++ = value;
}

void memcpy(void* dst_, const void* src_, uint32_t size) {
  ASSERT(dst_ != NULL && src_ != NULL);
  uint8_t* dst = (uint8_t*)dst_;
  const uint8_t* src = (const uint8_t*)src_;
  while (size-- > 0) *dst++ = *src++;
}

int memcmp(const void* a_, const void* b_, uint32_t size) {
  const char* a = a_;
  const char* b = b_;
  while (size-- > 0) {
    if (*a != *b) {
      return *a > *b ? 1 : -1;
    }
    a++;
    b++;
  }
  return 0;
}

char* strcpy(char* dst_, const char* src_) {
  ASSERT(dst_ != NULL && src_ != NULL);
  char* r = dst_;
  while ((*dst_++ = *src_++))
    ;
  return r;
}

uint32_t strlen(const char* str) {
  ASSERT(str != NULL);
  const char* p = str;
  while (*p++)
    ;
  return p - str - 1;
}

int8_t strcmp(const char* a, const char* b) {
  ASSERT(a != NULL && b != NULL);
  while (*a != 0 && *a == *b) {
    a++;
    b++;
  }
  return *a < *b ? -1 : *a > *b;
}

char* strchr(const char* str, const uint8_t ch) {
  ASSERT(str != NULL);
  while (*str != '\0') {
    if (*str == ch) {
      return (char*)str;
    }
    str++;
  }
  return NULL;
}

char* strrchr(const char* str, const uint8_t ch) {
  ASSERT(str != NULL);
  const char* last_ch = NULL;
  while (*str != '\0') {
    if (*str == ch) {
      last_ch = str;
    }
    str++;
  }
  return (char*)last_ch;
}

char* strcat(char* dst_, const char* src_) {
  ASSERT(dst_ != NULL && src_ != NULL);
  char* dst = dst_;
  char* src = src_;
  while (*dst != 0) dst++;
  while ((*dst++ = *src++))
    ;
  return dst_;
}

uint32_t strchrs(const char* str, uint8_t ch) {
  int cnt = 0;
  const char* p = str;
  while (*p) {
    if (*p == ch) {
      cnt++;
    }
    p++;
  }
  return cnt;
}
