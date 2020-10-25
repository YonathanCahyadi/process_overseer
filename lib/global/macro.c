#include "macro.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

/**
 * @brief  Check the memory allocation status
 * @note   on failed it will exit the program
 * @param  ptr: 
 * @return None
 */
void CHECK_MEM_ALLOCATION(void* ptr) {
	if (ptr == NULL) {
		fprintf(stderr, "%s\n", "Memory allocation failed!");
		exit(1);
	}
}

/**
 * @brief  Check if NULL
 * @note   
 * @param  ptr: pointer to be checked
 * @param  err_msg: error message to be printed in case of NULL
 * @return None
 */
void CHECK_IF_NULL(void* ptr, char* err_msg) {
	if (ptr == NULL) {
		fprintf(stderr, "%s\n", err_msg);
		exit(1);
	}
}

/**
 * @brief  Check the Size of nbyte with the expected size
 * @note   
 * @param  nbyte: n byte to be checked
 * @param  expected_size: the expected size in byte
 * @param  err_msg: error message in case the nbyte is not equals to expected size
 * @return None
 */
void SIZE_CHECK(int nbyte, int expected_size, char* err_msg) {
	if (nbyte != expected_size) {
		fprintf(stderr, "%s\n", err_msg);
		return;
	}
}

/**
 * @brief  Clear a buffer of char
 * @note   
 * @param  ptr: the pointer to char buffer
 * @param  len: the len of the buffer
 * @return None
 */
void CLEAR_CHAR_BUFFER(char* ptr, int len) {
	memset(ptr, '\0', len);
}

/**
 * @brief  this will set the memory allocated for the struct to 0
 * @note   
 * @param  ptr: pointer to the struct
 * @param  len: the size of the struct
 * @return None
 */
void CLEAR_STRUCT(void* ptr, int len) {
	memset(ptr, 0, len);
}

/**
 * @brief  check if i is equals to -1
 * @note   this function will check if i equals to -1, 
 * 		   if it is equals print err msg and exit with status code 1
 * 		   if it not equals return i
 * @param  i: the value to check
 * @param  err_msg_format: the format of errror message
 * @return i
 */
int CHECK(int i, char* err_msg_format, ...) {
	/** get the arguments */
	va_list args;
	va_start(args, err_msg_format);
	char msg_buffer[MAX_BUFFER_DEFAULT];
	CLEAR_CHAR_BUFFER(msg_buffer, MAX_BUFFER_DEFAULT);
	vsprintf(msg_buffer, err_msg_format, args);
	va_end(args);
	/** check if successful or failed */
	if (i == -1) {
		fprintf(stderr, "%s\n", msg_buffer);
		exit(1);
	}
	return i;
}

/**
 * @brief  check guard if its equals -1, also check if its interupted by signal
 * @note   
 * @param  i: the function return value
 * @param  err_msg_format: err msg format in case of error 
 * @return i 
 */
int CHECK2(int i, char* err_msg_format, ...){
	/** get the arguments */
	va_list args;
	va_start(args, err_msg_format);
	char msg_buffer[MAX_BUFFER_DEFAULT];
	CLEAR_CHAR_BUFFER(msg_buffer, MAX_BUFFER_DEFAULT);
	vsprintf(msg_buffer, err_msg_format, args);
	va_end(args);


	/** check if interupted by signal */
	if(errno == EINTR){ 
		return 0;
	}else if(i == -1){ /** check if successful or failed */
		fprintf(stderr, "%s\n", msg_buffer);
		exit(1);
	} 
	

	return i;
}



/**
 * @brief  Check if string is a float
 * @note   
 * @param  *str: the string
 * @return return 1 if valid, return 0 if invalid 
 */
int IS_FLOAT(const char *str){
    int len;
    float dummy = 0.0;
    if (sscanf(str, "%f %n", &dummy, &len) == 1 && len == (int)strlen(str))
        return 1; /** a valid float */
    else
        return 0; /** an invalid float */
}

/**
 * @brief  Check if string is interger
 * @note   
 * @param  *str: the string
 * @return return 1 if valid, return 0 if invalid
 */
int IS_INTERGER(const char* str) {
	char buffer[MAX_BUFFER_DEFAULT];
	CLEAR_CHAR_BUFFER(buffer, MAX_BUFFER_DEFAULT);
	snprintf(buffer, MAX_BUFFER_DEFAULT, "%d", atoi(str));
	return strcmp(buffer, str) == 0;
}

/**
 * @brief  guard for exec* family function
 * @note   
 * @return None
 */
void EXEC(int (*exec)(const char*, char* const*), char * const* argv, void (*callback)(FILE*, char*, ...), char *args){
	
	if(exec(argv[0], argv) == -1){
		callback(stderr, "could not execute %s", args);
		exit(1);
	}
}