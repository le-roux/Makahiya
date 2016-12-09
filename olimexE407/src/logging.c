#include "logging.h"
#include "utils.h"
#include "serial_user.h"

THD_WORKING_AREA(logging_wa, LOGGING_WA_SIZE);
uint8_t logging_buffer;

THD_FUNCTION(logging, arg) {
    UNUSED(arg);
    while (TRUE) {
        sdRead(&SD3, &logging_buffer, 1);
        chSequentialStreamPut((BaseSequentialStream*)&SDU1, logging_buffer);
    }
}
