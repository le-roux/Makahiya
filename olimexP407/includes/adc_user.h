#ifndef ADC_USER_H
#define ADC_USER_H
#include "hal.h"

#define ADC_GRP1_NUM_CHANNELS 1
#define ADC_GRP1_BUF_DEPTH 1

extern const ADCConversionGroup adc_conversion_config;

/** @brief Callback to periodically start the adc conversion.
 *
 * @param param UNUSED
 */
void adc_vt_cb(void* param);

#endif // ADC_USER_H
