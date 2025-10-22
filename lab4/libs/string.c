#include "uart.h"
#include "string.h"
#include "system_call.h"

static void vprintf(const char* fmt,__builtin_va_list args);
static void uvprintf(const char* fmt, __builtin_va_list args);

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

void printf(const char* fmt,...)
{
	__builtin_va_list args;
	__builtin_va_start(args, fmt);
	vprintf(fmt, args);
	__builtin_va_end(args);
}

static void vprintf(const char* fmt,__builtin_va_list args)
{
	while ((*fmt) != '\0') {
		if ((*fmt) == '%') {
			fmt ++;
			if ((*fmt) == '\0') {
				break;
			} else if((*fmt) == '%') {
				uart_putc('%');
				fmt ++;
			} else if((*fmt) == 'c') {
				char c = __builtin_va_arg(args, int);
				uart_putc(c);
				fmt ++;
			} else if((*fmt) == 'd') {
				int i = __builtin_va_arg(args, int);
				char temp[32] ={0};
				lld_to_str(i,temp);
				uart_puts(temp);
				fmt ++;	
			} else if((*fmt) == 's') {
				char* s = __builtin_va_arg(args, char*);
				uart_puts(s);
				fmt++;
			} else if((*fmt) == 'f') {
				float f = __builtin_va_arg(args, double);
				char temp[32] ={0};
				double_to_str(f,6,temp);
				uart_puts(temp);
				fmt++;
			} else if((*fmt) == 'u') {
				unsigned int u = __builtin_va_arg(args, unsigned int);
				char temp[32] ={0};
				llu_to_str(u,temp);
				uart_puts(temp);
				fmt++;
			} else if((*fmt) == 'x') {
				unsigned int u = __builtin_va_arg(args, unsigned int);
				char temp[32] ={0};
				llu_to_hex_str(u,temp, 0);
				uart_puts(temp);
				fmt++;
			} else if((*fmt) == 'X') {
				unsigned int u = __builtin_va_arg(args, unsigned int);
				char temp[32] ={0};
				llu_to_hex_str(u,temp, 1);
				uart_puts(temp);
				fmt++;
			} else if((*fmt) == 'l') {
				fmt++;

				if ((*fmt) == 'l') {
					fmt ++;	
					
					if ((*fmt) == 'd') {
						long long l = __builtin_va_arg(args, long long);
						char temp[64] ={0};	
						lld_to_str(l,temp);
						uart_puts(temp);
						fmt ++;	
					} else if((*fmt) == 'u') {
						unsigned long long llu = __builtin_va_arg(args, unsigned long long);
						char temp[64] ={0};
						llu_to_str(llu,temp);
						uart_puts(temp);
						fmt++;
					} else if ((*fmt) == 'x') {
						unsigned long long llu = __builtin_va_arg(args, unsigned long long);
						char temp[64] ={0};
						llu_to_hex_str(llu,temp, 0);
						uart_puts(temp);
						fmt++;
					} else if ((*fmt) == 'X') {
						unsigned long long llu = __builtin_va_arg(args, unsigned long long);
						char temp[64] ={0};
						llu_to_hex_str(llu,temp, 1);
						uart_puts(temp);
						fmt++;
					}
				} else if((*fmt) == 'f') {
					double d = __builtin_va_arg(args, double);
					char temp[64] ={0};
					double_to_str(d,15,temp);
					uart_puts(temp);
					fmt++;
				}
			}
		} else {
			uart_putc((*fmt));
			fmt++;
		}
	}
}

static int skip_whitespace(const char* str, int pos)
{
	while (str[pos] == ' ' || str[pos] == '\t' || str[pos] == '\n' || str[pos] == '\r') {
		pos++;
	}
	return pos;
}

static int str_to_int(const char* str, int* pos, int* result)
{
	int sign = 1;
	int value = 0;
	int start_pos = *pos;

	if (str[*pos] == '-') {
		sign = -1;
		(*pos)++;
	} else if (str[*pos] == '+') {
		(*pos)++;
	}

	if (str[*pos] < '0' || str[*pos] > '9') {
		*pos = start_pos;
		return 0;
	}

	while (str[*pos] >= '0' && str[*pos] <= '9') {
		value = value * 10 + (str[*pos] - '0');
		(*pos)++;
	}

	*result = value * sign;
	return 1;
}

static int str_to_char(const char* str, int* pos, char* result)
{
	if (str[*pos] == '\0') {
		return 0;
	}

	*result = str[*pos];
	(*pos)++;
	return 1;
}

static int str_to_string(const char* str, int* pos, char* result, int max_len)
{
	int i = 0;

	while (str[*pos] != ' ' && str[*pos] != '\t' && str[*pos] != '\n' &&
		   str[*pos] != '\r' && str[*pos] != '\0' && i < max_len - 1) {
		result[i] = str[*pos];
		i++;
		(*pos)++;
	}

	result[i] = '\0';
	return i > 0 ? 1 : 0;
}

static int str_to_hex(const char* str, int* pos, unsigned int* result)
{
	unsigned int value = 0;
	int start_pos = *pos;
	int digit_count = 0;

	while (str[*pos] != '\0') {
		char c = str[*pos];
		int digit_value = -1;

		if (c >= '0' && c <= '9') {
			digit_value = c - '0';
		} else if (c >= 'a' && c <= 'f') {
			digit_value = c - 'a' + 10;
		} else if (c >= 'A' && c <= 'F') {
			digit_value = c - 'A' + 10;
		} else {
			break;
		}

		value = (value << 4) | digit_value;
		(*pos)++;
		digit_count++;
	}

	if (digit_count == 0) {
		*pos = start_pos;
		return 0;
	}

	*result = value;
	return 1;
}

static int str_to_lld(const char* str, int* pos, long long* result)
{
	int sign = 1;
	long long value = 0;
	int start_pos = *pos;

	if (str[*pos] == '-') {
		sign = -1;
		(*pos)++;
	} else if (str[*pos] == '+') {
		(*pos)++;
	}

	if (str[*pos] < '0' || str[*pos] > '9') {
		*pos = start_pos;
		return 0;
	}

	while (str[*pos] >= '0' && str[*pos] <= '9') {
		value = value * 10 + (str[*pos] - '0');
		(*pos)++;
	}

	*result = value * sign;
	return 1;
}

static int str_to_llu(const char* str, int* pos, unsigned long long* result)
{
	unsigned long long value = 0;
	int start_pos = *pos;

	if (str[*pos] == '+') {
		(*pos)++;
	}

	if (str[*pos] < '0' || str[*pos] > '9') {
		*pos = start_pos;
		return 0;
	}

	while (str[*pos] >= '0' && str[*pos] <= '9') {
		value = value * 10 + (str[*pos] - '0');
		(*pos)++;
	}

	*result = value;
	return 1;
}

static int str_to_llx(const char* str, int* pos, unsigned long long* result)
{
	unsigned long long value = 0;
	int start_pos = *pos;
	int digit_count = 0;

	while (str[*pos] != '\0') {
		char c = str[*pos];
		int digit_value = -1;

		if (c >= '0' && c <= '9') {
			digit_value = c - '0';
		} else if (c >= 'a' && c <= 'f') {
			digit_value = c - 'a' + 10;
		} else if (c >= 'A' && c <= 'F') {
			digit_value = c - 'A' + 10;
		} else {
			break;
		}

		value = (value << 4) | digit_value;
		(*pos)++;
		digit_count++;
	}

	if (digit_count == 0) {
		*pos = start_pos;
		return 0;
	}

	*result = value;
	return 1;
}

static void read_input_line(char* buffer, int max_len)
{
	int i = 0;
	char c;

	while (i < max_len - 1) {
		c = uart_getc();

		if (c == '\r' || c == '\n') {
			uart_putc('\n');
			break;
		} else if (c == '\b' || c == 127) {
			if (i > 0) {
				i--;
				uart_putc('\b');
				uart_putc(' ');
				uart_putc('\b');
			}
		} else if (c >= 32 && c <= 126) {
			buffer[i] = c;
			uart_putc(c);
			i++;
		}
	}

	buffer[i] = '\0';
}

int simple_scanf(const char* fmt, ...)
{
	char input_buffer[256];
	__builtin_va_list args;
	__builtin_va_start(args, fmt);

	read_input_line(input_buffer, sizeof(input_buffer));

	int fmt_pos = 0;
	int input_pos = 0;
	int matched = 0;

	while (fmt[fmt_pos] != '\0') {
		input_pos = skip_whitespace(input_buffer, input_pos);

		if (fmt[fmt_pos] == '%') {
			fmt_pos++;

			if (fmt[fmt_pos] == '\0') {
				break;
			}

			switch (fmt[fmt_pos]) {
				case 'd': {
					int* int_ptr = __builtin_va_arg(args, int*);
					int value;
					if (str_to_int(input_buffer, &input_pos, &value)) {
						*int_ptr = value;
						matched++;
					} else {
						goto end;
					}
					break;
				}
				case 'c': {
					char* char_ptr = __builtin_va_arg(args, char*);
					char value;
					if (str_to_char(input_buffer, &input_pos, &value)) {
						*char_ptr = value;
						matched++;
					} else {
						goto end;
					}
					break;
				}
				case 's': {
					char* str_ptr = __builtin_va_arg(args, char*);
					if (str_to_string(input_buffer, &input_pos, str_ptr, 256)) {
						matched++;
					} else {
						goto end;
					}
					break;
				}
				case 'x': {
					unsigned int* uint_ptr = __builtin_va_arg(args, unsigned int*);
					unsigned int value;
					if (str_to_hex(input_buffer, &input_pos, &value)) {
						*uint_ptr = value;
						matched++;
					} else {
						goto end;
					}
					break;
				}
				case 'l': {
					fmt_pos++;
					if (fmt[fmt_pos] == 'l') {
						fmt_pos++;
						if (fmt[fmt_pos] == 'd') {
							long long* lld_ptr = __builtin_va_arg(args, long long*);
							long long value;
							if (str_to_lld(input_buffer, &input_pos, &value)) {
								*lld_ptr = value;
								matched++;
							} else {
								goto end;
							}
						} else if (fmt[fmt_pos] == 'u') {
							unsigned long long* llu_ptr = __builtin_va_arg(args, unsigned long long*);
							unsigned long long value;
							if (str_to_llu(input_buffer, &input_pos, &value)) {
								*llu_ptr = value;
								matched++;
							} else {
								goto end;
							}
						} else if (fmt[fmt_pos] == 'x') {
							unsigned long long* llx_ptr = __builtin_va_arg(args, unsigned long long*);
							unsigned long long value;
							if (str_to_llx(input_buffer, &input_pos, &value)) {
								*llx_ptr = value;
								matched++;
							} else {
								goto end;
							}
						} else {
							goto end;
						}
					} else {
						goto end;
					}
					break;
				}
				case '%':
					if (input_buffer[input_pos] == '%') {
						input_pos++;
					} else {
						goto end;
					}
					break;
				default:
					goto end;
			}

			fmt_pos++;
		} else if (fmt[fmt_pos] == ' ' || fmt[fmt_pos] == '\t' ||
				   fmt[fmt_pos] == '\n' || fmt[fmt_pos] == '\r') {
			input_pos = skip_whitespace(input_buffer, input_pos);
			fmt_pos++;
		} else {
			if (input_buffer[input_pos] == fmt[fmt_pos]) {
				input_pos++;
				fmt_pos++;
			} else {
				goto end;
			}
		}
	}

end:
	__builtin_va_end(args);
	return matched;
}

void memset(void *s, int c, unsigned int n)
{
	unsigned char *ptr = (unsigned char *)s;
	unsigned char value = (unsigned char)c;

	for (unsigned int i = 0; i < n; i++) {
		ptr[i] = value;
	}
}

static void uvprintf(const char* fmt, __builtin_va_list args)
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

void uprintf(const char* fmt, ...)
{
	__builtin_va_list args;
	__builtin_va_start(args, fmt);
	uvprintf(fmt, args);
	__builtin_va_end(args);
}