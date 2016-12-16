#ifndef UTILS_H
#define UTILS_H

#define UNUSED(x) (void)(x)
#define DEBUG(x) chprintf((BaseSequentialStream*)&SDU1, (x));

void int_to_char(char* out, int i);

#endif
