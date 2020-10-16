#include "macro.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

void CHECK_MEM_ALLOCATION(void* ptr) {
	if (ptr == NULL) {
		fprintf(stderr, "%s\n", "Memory allocation failed!");
		exit(1);
	}
}

void CHECK_IF_NULL(void* ptr, char* err_msg) {
	if (ptr == NULL) {
		fprintf(stderr, "%s\n", err_msg);
		exit(1);
	}
}



void SIZE_CHECK(int nbyte, int expected_size, char* err_msg) {
	if (nbyte != expected_size) {
		fprintf(stderr, "%s\n", err_msg);
		exit(1);
	}
}

void CLEAR_CHAR_BUFFER(char* ptr, int len) {
	memset(ptr, '\0', len);
}

void CLEAR_STRUCT(void* ptr, int len) {
	memset(ptr, 0, len);
}

int CHECK(int i, char* err_msg_format, ...) {
	/** get the arguments */
	va_list args;
	va_start(args, err_msg_format);
	char msg_buffer[MAX_BUFFER_DEFAULT];
	CLEAR_CHAR_BUFFER(msg_buffer, MAX_BUFFER_DEFAULT);
	vsprintf(msg_buffer, err_msg_format, args);
	va_end(args);
	/** check if successfull or failed */
	if (i == -1) {
		fprintf(stderr, "%s\n", msg_buffer);
		exit(1);
	}
	return i;
}

int IS_INTERGER(char* str) {
	char buffer[MAX_BUFFER_DEFAULT];
	CLEAR_CHAR_BUFFER(buffer, MAX_BUFFER_DEFAULT);
	snprintf(buffer, MAX_BUFFER_DEFAULT, "%d", atoi(str));
	return strcmp(buffer, str) == 0;
}

int IS_EQUALS(char* str1, char* str2){
	return strncmp(str1, str2, strlen(str2) + 1) == 0;
}

void EXEC(int (*exec)(const char*, char* const*), char * const* argv, void (*callback)(FILE*, char*, ...), char *args){
	
	if(exec(argv[0], argv) == -1){
		callback(stderr, "could not execute %s", args);
		exit(1);
	}
}