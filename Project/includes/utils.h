#ifndef UTILS_H
#define UTILS_H

#define UNUSED(x) (void)(x)
#define DEBUG(x,...) chprintf((BaseSequentialStream*)&RTTD, (x "\r\n"), ##__VA_ARGS__)
#define ABS(x) (x>=0)?(x):-(x)

void int_to_char(char* out, int i);

#endif
