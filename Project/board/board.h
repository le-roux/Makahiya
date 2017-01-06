/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef BOARD_H
#define BOARD_H

/*
 * Setup for Makahiya board.
 */

/*
 * Board identifier.
 */
#define BOARD_MAKAHIYA
#define BOARD_NAME                  "Makahiya"

/*
 * Board oscillators-related settings.
 */
#if !defined(STM32_LSECLK)
#define STM32_LSECLK                32768U
#endif

#if !defined(STM32_HSECLK)
#define STM32_HSECLK                40000000U
#endif

/*
 * Board voltages.
 * Required for performance limits calculation.
 */
#define STM32_VDD                   330U

/*
 * MCU type as defined in the ST header.
 */
#define STM32F405xx

/*
 * IO pins assignments.
 */
#define GPIOA_BLUETOOTH_UART_RTS    0U
#define GPIOA_BLUETOOTH_UART_CTS    1U
#define GPIOA_BLUETOOTH_UART_RX     2U
#define GPIOA_BLUETOOTH_UART_TX     3U
#define GPIOA_SERVO1                4U
#define GPIOA_SERVO2                5U
#define GPIOA_SERVO3                6U
#define GPIOA_SERVO4                7U
#define GPIOA_LED4_G                8U
#define GPIOA_WIFI_RX               9U
#define GPIOA_WIFI_TX               10U
#define GPIOA_WIFI_RTS              11U
#define GPIOA_WIFI_CTS              12U
#define GPIOA_SWDIO                 13U
#define GPIOA_SWCLK                 14U
#define GPIOA_WIFI_RST              15U

#define GPIOB_LED1_G                0U
#define GPIOB_LED1_B                1U
#define GPIOB_LED2_R                2U
#define GPIOB_LED5_G                3U
#define GPIOB_LEDHP_R               4U
#define GPIOB_LEDHP_G               5U
#define GPIOB_LEDHP_B               6U
#define GPIOB_LEDHP_W               7U
#define GPIOB_FDC_I2C_SCL           8U
#define GPIOB_FDC_I2C_SDA           9U
#define GPIOB_LED2_G                10U
#define GPIOB_LED2_B                11U
#define GPIOB_AUDIO_LRCK            12U
#define GPIOB_AUDIO_SCLK            13U
#define GPIOB_LED3_R                14U
#define GPIOB_AUDIO_SDIN            15U

#define GPIOC_BLUETOOTH_SET_MASTER  0U
#define GPIOC_BLUETOOTH_FACTORY_RST 1U
#define GPIOC_BLUETOOTH_AUTO_DISCOV 2U
#define GPIOC_BLUETOOTH_RST         3U
#define GPIOC_SERVO5                4U
#define GPIOC_LED1_R                5U
#define GPIOC_AUDIO_MCLK            6U
#define GPIOC_LED3_G                7U
#define GPIOC_LED3_B                8U
#define GPIOC_LED4_R                9U
#define GPIOC_FDC1_INT              10U
#define GPIOC_LED4_B                11U
#define GPIOC_LED5_R                12U
#define GPIOC_FDC2_INT              13U
#define GPIOC_FDC1_SHUTDOWN         14U
#define GPIOC_FDC2_SHUTDOWN         15U

#define GPIOD_LED5_B                2U

#define GPIOH_OSC_IN                0U
#define GPIOH_OSC_OUT               1U

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 * Please refer to the STM32 Reference Manual for details.
 */
#define PIN_MODE_INPUT(n)           (0U << ((n) * 2))
#define PIN_MODE_OUTPUT(n)          (1U << ((n) * 2))
#define PIN_MODE_ALTERNATE(n)       (2U << ((n) * 2))
#define PIN_MODE_ANALOG(n)          (3U << ((n) * 2))
#define PIN_OTYPE_PUSHPULL(n)       (0U << (n))
#define PIN_OTYPE_OPENDRAIN(n)      (1U << (n))
#define PIN_OSPEED_2M(n)            (0U << ((n) * 2))
#define PIN_OSPEED_25M(n)           (1U << ((n) * 2))
#define PIN_OSPEED_50M(n)           (2U << ((n) * 2))
#define PIN_OSPEED_100M(n)          (3U << ((n) * 2))
#define PIN_PUDR_FLOATING(n)        (0U << ((n) * 2))
#define PIN_PUDR_PULLUP(n)          (1U << ((n) * 2))
#define PIN_PUDR_PULLDOWN(n)        (2U << ((n) * 2))
#define PIN_AFIO_AF(n, v)           ((v##U) << (((n) % 8) * 4))

#define VAL_GPIOA_MODER             (PIN_MODE_ALTERNATE(GPIOA_SWDIO) |   \
				     PIN_MODE_ALTERNATE(GPIOA_SWCLK))
#define VAL_GPIOA_OTYPER            0x00000000
#define VAL_GPIOA_OSPEEDR           0xFFFFFFFF
#define VAL_GPIOA_PUPDR             (PIN_PUDR_PULLDOWN(GPIOA_SWCLK))
#define VAL_GPIOA_ODR               0xFFFFFFFF 
#define VAL_GPIOA_AFRL              0x00000000
#define VAL_GPIOA_AFRH              (PIN_AFIO_AF(GPIOA_SWDIO, 0) |       \
				     PIN_AFIO_AF(GPIOA_SWCLK, 0))

#define VAL_GPIOB_MODER             (0)
#define VAL_GPIOA_OTYPER            0x00000000
#define VAL_GPIOA_OSPEEDR           0xFFFFFFFF
#define VAL_GPIOA_PUPDR             (0)
#define VAL_GPIOA_ODR               0xFFFFFFFF 
#define VAL_GPIOA_AFRL              0x00000000
#define VAL_GPIOA_AFRH              0x00000000

#define VAL_GPIOC_MODER             (0)
#define VAL_GPIOA_OTYPER            0x00000000
#define VAL_GPIOA_OSPEEDR           0xFFFFFFFF
#define VAL_GPIOA_PUPDR             (0)
#define VAL_GPIOA_ODR               0xFFFFFFFF 
#define VAL_GPIOA_AFRL              0x00000000
#define VAL_GPIOA_AFRH              0x00000000

#define VAL_GPIOD_MODER             (0)
#define VAL_GPIOA_OTYPER            0x00000000
#define VAL_GPIOA_OSPEEDR           0xFFFFFFFF
#define VAL_GPIOA_PUPDR             (0)
#define VAL_GPIOA_ODR               0xFFFFFFFF 
#define VAL_GPIOA_AFRL              0x00000000
#define VAL_GPIOA_AFRH              0x00000000

#define VAL_GPIOH_MODER             (PIN_MODE_INPUT(GPIOH_OSC_IN) |     \
				     PIN_MODE_INPUT(GPIOH_OSC_OUT))
#define VAL_GPIOA_OTYPER            0x00000000
#define VAL_GPIOA_OSPEEDR           0xFFFFFFFF
#define VAL_GPIOA_PUPDR             (PIN_PUDR_FLOATING(GPIOH_OSC_IN) |  \
				     PIN_PUDR_FLOATING(GPIOH_OSC_OUT))
#define VAL_GPIOA_ODR               0xFFFFFFFF 
#define VAL_GPIOA_AFRL              0x00000000
#define VAL_GPIOA_AFRH              0x00000000

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* BOARD_H */
