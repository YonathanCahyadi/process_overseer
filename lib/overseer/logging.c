#include "logging.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "../global/macro.h"

/**
 * @brief  print a message in the log format
 * @note   This will print message in the log format:
 * 		   %Y-%m-%d %H:%M:%S - <message>
 * @param  *type: the log type [stdout, stderr]
 * @param  *format: the message format
 * @return None
 */
void print_log(FILE *type, char *format, ...) {
	time_t timer;
	CLEAR_STRUCT(&timer, sizeof(timer));
	char time_buffer[TIME_FORMAT_LEN];
	CLEAR_CHAR_BUFFER(time_buffer, TIME_FORMAT_LEN);
	struct tm *tm_info = NULL;

	/** get the current time */
	timer = time(NULL);
	tm_info = localtime(&timer);

	/** format the current time */
	strftime(time_buffer, sizeof(time_buffer), TIME_FORMAT, tm_info);

	/** get the arguments */
	va_list args;
	va_start(args, format);
	char msg_buffer[MAX_MSG_LEN];
	CLEAR_CHAR_BUFFER(msg_buffer, MAX_MSG_LEN);
	vsprintf(msg_buffer, format, args);
	va_end(args);

	/** clear the stdout and stderr buffer */
	fflush(stdout);
	fflush(stderr);

	/** print the formatted time along side the msg */
	fprintf(type, "%s - %s\n", time_buffer, msg_buffer);
}