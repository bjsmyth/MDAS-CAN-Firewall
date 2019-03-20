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

#ifndef __MDAS_CAN_H__
#define __MDAS_CAN_H__

//Defines
#define VEHICLE_STEERING (1)
#define VEHICLE_BRAKE (2)
#define VEHICLE_THROTTLE (3)
#define CONTROLLER_STEERING (4)
#define CONTROLLER_BRAKE (5)
#define CONTROLLER_THROTTLE (6)

//Initializers
void CAN0_Init(uint32_t baud);
void CAN1_Init(uint32_t baud);

void CAN_Init_MsgObj();

//Interrupt handlers
void CAN0_IntHdlr();
void CAN1_IntHdlr();

#endif //_MDAS_CAN_H
