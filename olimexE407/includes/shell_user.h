#ifndef SHELL_H
#define SHELL_H

#include "ch.h"
#include "hal.h"
#include "shell.h"

extern thread_t* shelltp;
extern const ShellCommand commands[];
extern const ShellConfig shell_cfg1;

#define SHELL_WA_SIZE THD_WORKING_AREA_SIZE(2048)

#endif
