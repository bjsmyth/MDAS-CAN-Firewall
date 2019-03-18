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
#include "inc/hw_ints.h"

#ifndef __MDAS_CAN_H__
#define __MDAS_CAN_H__

enum FromVehicle {
  FROM_STEERING = 1,
  FROM_BRAKE = 2,
  FROM_THROTTLE = 3
};

enum ToVehicle {
  TO_STEERING = 4,
  TO_BRAKE = 5,
  TO_THROTTLE = 6,
};

//Initializers
void CAN0_Init(uint32_t baud);
void CAN1_Init(uint32_t baud);

void CAN_Init_MsgObj();

//Interrupt handlers
void CAN0_IntHdlr();
void CAN1_IntHdlr();

#endif //_MDAS_CAN_H
