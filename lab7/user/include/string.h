#ifndef _STRING_UAPI_H
#define _STRING_UAPI_H

int strcmp(const char* str1, const char* str2);

void llu_to_hex_str(unsigned long long llu, char* str, int upcase);

void llu_to_str(unsigned long long llu, char* str);

void lld_to_str(long long lld, char* str);

void double_to_str(double ul,int precision, char* str);

void reverse_str(char* str, int len);

void printf(const char* fmt, ...);

void memset(void *s, int c, unsigned int n);
void *memcpy(void *dest, const void *src, unsigned int n);

#endif