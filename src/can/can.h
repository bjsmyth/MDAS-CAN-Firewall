#include <stdint.h>
#include <stdbool.h>

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/can.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"
#include "inc/hw_can.h"
#include "inc/hw_ints.h"

#ifndef _MDAS_CAN_H
#define _MDAS_CAN_H

enum FromVehicle {
  FROM_STEERING = 0,
  FROM_BRAKE = 1,
  FROM_THROTTLE = 2
};

//Initializers
void CAN0_Init(uint32_t baud);
void CAN1_Init(uint32_t baud);

//Interrupt handlers
void CAN0_IntHdlr();

void CAN0_To_CAN1(tCANMsgObject toForward);

#endif //_MDAS_CAN_H
