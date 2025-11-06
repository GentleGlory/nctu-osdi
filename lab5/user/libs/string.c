#include "type.h"
#include "string.h"
#include "system_call.h"

static void vprintf(const char* fmt, __builtin_va_list args);

int strcmp(const char* str1, const char* str2)
{
	while (*str1 && (*str1 == *str2)) {
		str1++;
		str2++;
	}
	
	return (unsigned char)*str1 - (unsigned char)*str2;
}

void llu_to_hex_str(unsigned long long llu, char* str,int upcase)
{
	char hex[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
	char hex_upcase[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

	int cnt = 0;
	while (llu) {
		str[cnt] = upcase ? hex_upcase[(llu & 0xf)] : hex[(llu & 0xf)];
		llu >>= 4;
		cnt++;
	}

	if (cnt) {
		reverse_str(str,cnt);
	} else {
		str[0] = '0';
		cnt++;
	}

	str[cnt] = '\0';
}

void llu_to_str(unsigned long long llu, char* str)
{
	int cnt = 0;
	while (llu) {
		str[cnt] = '0' + (llu % 10);
		llu /= 10;
		cnt++;	
	}

	if (cnt) {
		reverse_str(str,cnt);
	} else {
		str[0] = '0';
		cnt++;
	}	

	str[cnt] = '\0';
}

void lld_to_str(long long lld, char* str)
{
	int cnt = 0;
	int negative = 0;

	if (lld < 0) {
		str[0] = '-';
		cnt++;
		negative = 1;
		lld = -lld;
	}

	while (lld) {
		str[cnt] = '0' + (lld % 10);
		lld /= 10;
		cnt++;
	}

	if (cnt == (negative ? 1 : 0)) {
		str[cnt] = '0';
		cnt++;
	}

	// Only reverse the digit part, keep the negative sign at the front
	if (negative) {
		reverse_str(&str[1], cnt - 1);
	} else {
		reverse_str(str, cnt);
	}

	str[cnt] = '\0';
}

void reverse_str(char* str, int len)
{
	int left = 0, righ = len - 1;

	while (left < righ) {
		char c = str[left];
		str[left] = str[righ];
		str[righ] = c;
		left ++;
		righ --;
	}
}

void double_to_str(double ul,int precision, char* str)
{
	// Handle the sign
	int pos = 0;
	if (ul < 0) {
		str[pos++] = '-';
		ul = -ul; // Make the value positive
	}

	// Extract the integer part
	long long intPart = (long long)ul;
	double fractionalPart = ul - intPart;

	// Convert integer part to string
	char intBuffer[32];
	int intLen = 0;
	do {
		intBuffer[intLen++] = '0' + (intPart % 10);
		intPart /= 10;
	} while (intPart > 0);

	// Reverse and copy integer part to the main buffer
	for (int i = intLen - 1; i >= 0; i--) {
		str[pos++] = intBuffer[i];
	}

	// Add the decimal point
	str[pos++] = '.';

	// Convert fractional part to string
	for (int i = 0; i < precision; i++) {
		fractionalPart *= 10;
		int digit = (int)fractionalPart;
		str[pos++] = '0' + digit;
		fractionalPart -= digit;
	}

	str[pos] = '\0';  // Null-terminate the string
}

void memset(void *s, int c, unsigned int n)
{
	unsigned char *ptr = (unsigned char *)s;
	unsigned char value = (unsigned char)c;

	for (unsigned int i = 0; i < n; i++) {
		ptr[i] = value;
	}
}

void *memcpy(void *dest, const void *src, unsigned int n)
{
	unsigned char *d = (unsigned char *)dest;
	const unsigned char *s = (const unsigned char *)src;

	for (unsigned int i = 0; i < n; i++) {
		d[i] = s[i];
	}

	return dest;
}

static void vprintf(const char* fmt, __builtin_va_list args)
{
	char buffer[1024] = {0};
	int buf_pos = 0;

	while ((*fmt) != '\0' && buf_pos < 1023) {
		if ((*fmt) == '%') {
			fmt++;
			if ((*fmt) == '\0') {
				break;
			} else if ((*fmt) == '%') {
				buffer[buf_pos++] = '%';
				fmt++;
			} else if ((*fmt) == 'c') {
				char c = (char)__builtin_va_arg(args, int);
				buffer[buf_pos++] = c;
				fmt++;
			} else if ((*fmt) == 'd') {
				int i = __builtin_va_arg(args, int);
				char temp[32] = {0};
				lld_to_str(i, temp);
				int j = 0;
				while (temp[j] != '\0' && buf_pos < 1023) {
					buffer[buf_pos++] = temp[j++];
				}
				fmt++;
			} else if ((*fmt) == 's') {
				char* s = __builtin_va_arg(args, char*);
				int j = 0;
				while (s[j] != '\0' && buf_pos < 1023) {
					buffer[buf_pos++] = s[j++];
				}
				fmt++;
			} else if ((*fmt) == 'f') {
				double f = __builtin_va_arg(args, double);
				char temp[32] = {0};
				double_to_str(f, 6, temp);
				int j = 0;
				while (temp[j] != '\0' && buf_pos < 1023) {
					buffer[buf_pos++] = temp[j++];
				}
				fmt++;
			} else if ((*fmt) == 'u') {
				unsigned int u = __builtin_va_arg(args, unsigned int);
				char temp[32] = {0};
				llu_to_str(u, temp);
				int j = 0;
				while (temp[j] != '\0' && buf_pos < 1023) {
					buffer[buf_pos++] = temp[j++];
				}
				fmt++;
			} else if ((*fmt) == 'x') {
				unsigned int u = __builtin_va_arg(args, unsigned int);
				char temp[32] = {0};
				llu_to_hex_str(u, temp, 0);
				int j = 0;
				while (temp[j] != '\0' && buf_pos < 1023) {
					buffer[buf_pos++] = temp[j++];
				}
				fmt++;
			} else if ((*fmt) == 'X') {
				unsigned int u = __builtin_va_arg(args, unsigned int);
				char temp[32] = {0};
				llu_to_hex_str(u, temp, 1);
				int j = 0;
				while (temp[j] != '\0' && buf_pos < 1023) {
					buffer[buf_pos++] = temp[j++];
				}
				fmt++;
			} else if ((*fmt) == 'l') {
				fmt++;

				if ((*fmt) == 'l') {
					fmt++;

					if ((*fmt) == 'd') {
						long long l = __builtin_va_arg(args, long long);
						char temp[64] = {0};
						lld_to_str(l, temp);
						int j = 0;
						while (temp[j] != '\0' && buf_pos < 1023) {
							buffer[buf_pos++] = temp[j++];
						}
						fmt++;
					} else if ((*fmt) == 'u') {
						unsigned long long llu = __builtin_va_arg(args, unsigned long long);
						char temp[64] = {0};
						llu_to_str(llu, temp);
						int j = 0;
						while (temp[j] != '\0' && buf_pos < 1023) {
							buffer[buf_pos++] = temp[j++];
						}
						fmt++;
					} else if ((*fmt) == 'x') {
						unsigned long long llu = __builtin_va_arg(args, unsigned long long);
						char temp[64] = {0};
						llu_to_hex_str(llu, temp, 0);
						int j = 0;
						while (temp[j] != '\0' && buf_pos < 1023) {
							buffer[buf_pos++] = temp[j++];
						}
						fmt++;
					} else if ((*fmt) == 'X') {
						unsigned long long llu = __builtin_va_arg(args, unsigned long long);
						char temp[64] = {0};
						llu_to_hex_str(llu, temp, 1);
						int j = 0;
						while (temp[j] != '\0' && buf_pos < 1023) {
							buffer[buf_pos++] = temp[j++];
						}
						fmt++;
					}
				} else if ((*fmt) == 'f') {
					double d = __builtin_va_arg(args, double);
					char temp[64] = {0};
					double_to_str(d, 15, temp);
					int j = 0;
					while (temp[j] != '\0' && buf_pos < 1023) {
						buffer[buf_pos++] = temp[j++];
					}
					fmt++;
				}
			}
		} else {
			buffer[buf_pos++] = (*fmt);
			fmt++;
		}
	}

	buffer[buf_pos] = '\0';
	system_call_uart_write(buffer, buf_pos);
}

void printf(const char* fmt, ...)
{
	__builtin_va_list args;
	__builtin_va_start(args, fmt);
	vprintf(fmt, args);
	__builtin_va_end(args);
}
