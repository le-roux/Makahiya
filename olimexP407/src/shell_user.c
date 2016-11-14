#include "shell_user.h"
#include "utils.h"
#include "usbcfg.h"
#include "chprintf.h"
#include "stdlib.h"
#include <string.h>
#include "web.h"

thread_t* shelltp = NULL;

void start(BaseSequentialStream* chp, int argc, char* argv[]) {
    UNUSED(chp);
    if (argc == 1) {
        pwmChangePeriod(&PWMD3, atoi(argv[0]));
    }
    pwmEnableChannel(&PWMD3, 3, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, 50));
}

void stop(BaseSequentialStream* chp, int argc, char* argv[]) {
    UNUSED(chp);
    UNUSED(argc);
    UNUSED(argv);
    pwmDisableChannel(&PWMD3, 3);
}

void web(BaseSequentialStream* chp, int argc, char* argv[]) {
    if (argc != 1) {
        chprintf(chp, "Usage: web hostname\r\n");
        return;
    }
    strcpy(hostname, argv[0]);
    chBSemSignal(&web_bsem);
}

void led(BaseSequentialStream* chp, int argc, char* argv[]) {
    if (argc != 3) {
        chprintf(chp, "Usage: led red green blue\r\n");
        return;
    }
    pwmEnableChannel(&PWMD4, 3, atoi(argv[0]));
    pwmEnableChannel(&PWMD1, 2, atoi(argv[1]));
    pwmEnableChannel(&PWMD3, 1, atoi(argv[2]));

}

void audio_start(BaseSequentialStream* chp, int argc, char* argv[]) {
    UNUSED(argc);
    UNUSED(argv);
    chprintf(chp, "i2s start exchange (state: %i)\r\n", I2SD3.state);
    i2sStartExchange(&I2SD3);
    chprintf(chp, "exchange started (state: %i)\r\n", I2SD3.state);
}

void audio_stop(BaseSequentialStream* chp, int argc, char* argv[]) {
    UNUSED(argc);
    UNUSED(argv);
    chprintf(chp, "i2s stop exchange (state: %i)\r\n", I2SD3.state);
    i2sStopExchange(&I2SD3);
    chprintf(chp, "exchange stopped (state: %i)\r\n", I2SD3.state);
}

const ShellCommand commands[] = {
  {"start", start},
  {"stop", stop},
  {"web", web},
  {"led", led},
  {"audio_start", audio_start},
  {"audio_stop", audio_stop},
  {NULL, NULL}
};

const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SDU1,
  commands
};
