#ifndef LIB_GLOBAL_MACRO_H_
#define LIB_GLOBAL_MACRO_H_

#include <stdio.h>

#define DEFAULT_ALLOCATION_SIZE 1
#define MAX_BUFFER_DEFAULT 10000

void CHECK_MEM_ALLOCATION(void* ptr);

int CHECK(int i, char* err_msg_format, ...);

void CLEAR_CHAR_BUFFER(char* ptr, int len);

void CHECK_IF_NULL(void* ptr, char* err_msg);

void CLEAR_STRUCT(void* ptr, int len);

void SIZE_CHECK(int nbyte, int expected_size, char* err_msg);

int IS_INTERGER(const char* str);

int IS_FLOAT(const char *str);

void EXEC(int (*exec)(const char*, char* const*), char * const* argv, void (*callback)(FILE*, char*, ...), char *args);

#endif