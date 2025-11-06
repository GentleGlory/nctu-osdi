#ifndef _USER_LIB_H
#define _USER_LIB_H

#include "type.h"
#include "string.h"

void print_timestamp();
void irq_test();
size_t uart_read(char buf[], size_t size);
size_t uart_write(const char buf[], size_t size);
int fork();
void exit(int status);
void delay(uint64_t ms);
int get_task_id();
void exc_test();

#endif