#ifndef ANKIT_ERROR
#define ANKIT_ERROR

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Prints a formatted (fmt) message, appends the system error
 * message (like perror), and exits.
 *
 * @param fmt The format string (like printf).
 * @param ... The arguments for the format string.
 */
void err_sys(const char *fmt, ...);

/**
 * @brief Prints a formatted (fmt) message and exits.
 *
 * @param fmt The format string (like printf).
 * @param ... The arguments for the format string.
 */
void err_quit(const char *fmt, ...);

#endif