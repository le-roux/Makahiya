#include "adc_user.h"
#include "pwm_user.h"
#include "tim_user.h"
#include "utils.h"

// The buffer where to write the values read by the ADC
static adcsample_t samples[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];

/** @brief Callback that update the PWM value when ADC conversion is finished.
 *
 */
static void adc_cb(ADCDriver* driver, adcsample_t* buffer, size_t n);

const ADCConversionGroup adc_conversion_config = {
    FALSE,                  // No circular buffer
    ADC_GRP1_NUM_CHANNELS,  // Number of Channels
    adc_cb,                 // Callback
    NULL,                   // Error Callback
    // End of mandatory fields
    0,                      // CR1
    ADC_CR2_SWSTART,        // CR2 - Single conversion mode selected
    ADC_SMPR1_SMP_AN10(ADC_SAMPLE_3), // SMPR1 - channel 10 selected
    0,                      // SMPR2
    ADC_SQR1_NUM_CH(ADC_GRP1_NUM_CHANNELS), // SQR1 - 1 conversion
    0,                      // SQR2
    ADC_SQR3_SQ1_N(ADC_CHANNEL_IN10) // SQR3
};

void adc_vt_cb(void* param) {
    UNUSED(param);
    chSysLockFromISR();
    adcStartConversionI(&ADCD1, &adc_conversion_config, samples, ADC_GRP1_BUF_DEPTH);
    chVTSetI(&adc_vt, MS2ST(100), adc_vt_cb, NULL);
    chSysUnlockFromISR();
}

static void adc_cb(ADCDriver* driver, adcsample_t* buffer, size_t n) {
    UNUSED(driver);
    UNUSED(n);
    chSysLockFromISR();
    pwmEnableChannelI(&PWMD13, 0, (buffer[0] / 4));
    chSysUnlockFromISR();
}
