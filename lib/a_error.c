#include "a_error.h"

/**
 * @brief Prints a formatted (fmt) message, appends the system error
 * message (like perror), and exits.
 *
 * @param fmt The format string (like printf).
 * @param ... The arguments for the format string.
 */
void err_sys(const char *fmt, ...){
    va_list args;
    char    buf[1024];
    int     len;

    // --- This is the "fmt" part ---
    va_start(args, fmt);
    // 1. Take the format string (fmt) and all the '...' arguments (args)
    // 2. Print them safely into the 'buf' buffer.
    vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);
    // --- End of "fmt" part ---

    // --- This is the "perror" part ---
    len = strlen(buf);
    // 1. Get the system error string (like perror does)
    // 2. Append it to our buffer.
    snprintf(buf + len, sizeof(buf) - len, ": %s", strerror(errno));
    // --- End of "perror" part ---

    // Print the combined message and exit
    fprintf(stderr, "%s\n", buf);
    exit(EXIT_FAILURE);
}

/**
 * @brief Prints a formatted (fmt) message and exits.
 *
 * @param fmt The format string (like printf).
 * @param ... The arguments for the format string.
 */
void err_quit(const char *fmt, ...){
    va_list args;

    // --- This is the "fmt" part ---
    va_start(args, fmt);
    // 1. Take the format string (fmt) and all the '...' arguments (args)
    // 2. Print them directly to stderr.
    vfprintf(stderr, fmt, args);
    va_end(args);
    // --- End of "fmt" part ---

    // Print a newline and exit
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}