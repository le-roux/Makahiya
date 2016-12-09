#include "shell_user.h"
#include "utils.h"
#include "usbcfg.h"
#include "chprintf.h"
#include "stdlib.h"
#include <string.h>
#include "serial_user.h"

thread_t* shelltp = NULL;

void servo(BaseSequentialStream* chp, int argc, char* argv[]) {
    if (argc != 1) {
        chprintf(chp, "Usage: servo angle [0-170]\r\n");
        return;
    }
    pwmEnableChannel(&PWMD1, 0, 70 + atoi(argv[0]));
}

void serial_start(BaseSequentialStream* chp, int argc, char* argv[]) {
    UNUSED(argc);
    UNUSED(argv);
    for (int i = 0; i < SERIAL_TX_BUFFER_SIZE; i++)
        serial_tx_buffer[i] = 'a' + (i % 26);
    sdStart(&SD6, &serial_cfg);
    sdWrite(&SD6, serial_tx_buffer, SERIAL_TX_BUFFER_SIZE);
}

void serial_stop(BaseSequentialStream* chp, int argc, char* argv[]) {
    UNUSED(chp);
    UNUSED(argc);
    UNUSED(argv);
    sdStop(&SD6);
}

const ShellCommand commands[] = {
  {"servo", servo},
  {"serial_start", serial_start},
  {"serial_stop", serial_stop},
  {NULL, NULL}
};

const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SDU1,
  commands
};
