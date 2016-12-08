#include "shell_user.h"
#include "utils.h"
#include "usbcfg.h"
#include "chprintf.h"
#include "stdlib.h"
#include <string.h>

thread_t* shelltp = NULL;

void servo(BaseSequentialStream* chp, int argc, char* argv[]) {
    if (argc != 1) {
        chprintf(chp, "Usage: servo angle [0-170]\r\n");
        return;
    }
    pwmEnableChannel(&PWMD1, 0, 70 + atoi(argv[0]));
}

const ShellCommand commands[] = {
  {"servo", servo},
  {NULL, NULL}
};

const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SDU1,
  commands
};
