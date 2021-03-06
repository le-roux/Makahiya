#ifndef UTILS_H
#define UTILS_H

#define UNUSED(x) (void)(x)
#define DEBUG(x,...) chprintf((BaseSequentialStream*)&RTTD, (x "\r\n"), ##__VA_ARGS__)
#define ABS(x) (x>=0)?(x):-(x)

/**
 * @brief Convert an integer to it's string representation.
 * The buffer given as parameter must be large enough to store the string.
 * The maximum value is 999999999.
 */
void int_to_char(char* out, int i);

#endif
