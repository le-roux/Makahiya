#ifndef UTILS_H
#define UTILS_H

#define UNUSED(x) (void)(x)
#define DEBUG(x,...) chprintf((BaseSequentialStream*)&SDU1, (x "\r\n"), ##__VA_ARGS__)
#define PUT(x) chSequentialStreamPut((BaseSequentialStream*)&SDU1, x)

void int_to_char(char* out, int i);

#endif
