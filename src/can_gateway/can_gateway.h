#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/can.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"
#include "inc/hw_can.h"
#include "inc/tm4c123gh6pm.h"
#include "../globals_macros/globals_macros.h"

#ifndef __MDAS_CAN_H__
#define __MDAS_CAN_H__

//Defines
#define CONTROLLER_BRAKE_ID (0x18DB0000)
#define CONTROLLER_STEERING_ID (0x19DB0000)
#define CONTROLLER_THROTTLE_ID (0x1ADB0000)
#define VEHICLE_FEEDBACK_ID (0x1CDBFFFF)

#define CONTROLLER_BRAKE (1)
#define CONTROLLER_STEERING (2)
#define CONTROLLER_THROTTLE (3)
#define VEHICLE_FEEDBACK (4)

//Initializers
void CAN0_Init(uint32_t baud);
void CAN1_Init(uint32_t baud);

void CAN_Init_MsgObj();

#endif //_MDAS_CAN_H
