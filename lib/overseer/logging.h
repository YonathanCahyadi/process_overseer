#ifndef LIB_OVERSEER_LOGGING_H_
#define LIB_OVERSEER_LOGGING_H_

#include <stdio.h>

#define TIME_FORMAT "%Y-%m-%d %H:%M:%S"
#define TIME_FORMAT_LEN 26
#define MAX_MSG_LEN 500

void print_log(FILE *type, char *format, ...);

#endif /** LIB_OVERSEER_LOGGING_H_ */