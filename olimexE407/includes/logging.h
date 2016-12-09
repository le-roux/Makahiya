#ifndef LOGGING_H
#define LOGGING_H

#include "ch.h"
#include "hal.h"
#include "usbcfg.h"

/***********************/
/*        Defines      */
/***********************/
#define LOGGING_WA_SIZE 128
#define LOGGING_BUFFER_SIZE 32

/***********************/
/*       Variables     */
/***********************/
extern THD_WORKING_AREA(logging_wa, LOGGING_WA_SIZE);
extern uint8_t logging_buffer;

/***********************/
/*       Functions     */
/***********************/
extern THD_FUNCTION(logging, arg);

#endif // LOGGING_H
