#include <stdint.h>
#include <stdbool.h>
#include "driverlib/gpio.h"

#ifndef __GLOBALS_MACROS_H__
#define __GLOBALS_MACROS_H__

#define LED_R (GPIO_PIN_1)
#define LED_G (GPIO_PIN_2)
#define LED_B (GPIO_PIN_3)

extern volatile uint32_t g_can0TxQueue, g_can1TxQueue;

#endif
