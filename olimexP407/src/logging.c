#include "logging.h"
#include "utils.h"
#include "serial_user.h"

THD_WORKING_AREA(logging_wa, LOGGING_WA_SIZE);
uint8_t logging_buffer[LOGGING_BUFFER_SIZE];
BSEMAPHORE_DECL(logging_bsem, true);

static int index;

THD_FUNCTION(logging, arg) {
    UNUSED(arg);
    while (TRUE) {
        index = 0;
        sdRead(&SD6, logging_buffer, LOGGING_BUFFER_SIZE);
        while ((logging_buffer[index] != '\0') & (index < LOGGING_BUFFER_SIZE - 1)) {
            chSequentialStreamPut((BaseSequentialStream*)&SDU1, logging_buffer[index]);
            index++;
        }
    }
}
