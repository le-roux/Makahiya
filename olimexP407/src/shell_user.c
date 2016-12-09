#include "shell_user.h"
#include "utils.h"
#include "usbcfg.h"
#include "chprintf.h"
#include "stdlib.h"
#include <string.h>
#include "usart.h"

thread_t* shelltp = NULL;

void servo(BaseSequentialStream* chp, int argc, char* argv[]) {
    if (argc != 1) {
        chprintf(chp, "Usage: servo angle [0-170]\r\n");
        return;
    }
    pwmEnableChannel(&PWMD1, 0, 70 + atoi(argv[0]));
}

void uart_start(BaseSequentialStream* chp, int argc, char* argv[]) {
    UNUSED(argc);
    UNUSED(argv);
    uartStartReceive(&UARTD3, 2, uart_rx_buffer);
    uartStartSend(&UARTD3, UART_BUF_SIZE, uart_tx_buffer);
    chprintf(chp, "foo\r\n");
}

void uart_stop(BaseSequentialStream* chp, int argc, char* argv[]) {
    UNUSED(chp);
    UNUSED(argc);
    UNUSED(argv);
    uartStopSend(&UARTD3);
    uartStopReceive(&UARTD3);
}

const ShellCommand commands[] = {
  {"servo", servo},
  {"uart_start", uart_start},
  {"uart_stop", uart_stop},
  {NULL, NULL}
};

const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SDU1,
  commands
};
