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
#include "inc/tm4c123gh6pm.h"

#ifndef __MDAS_CAN_H__
#define __MDAS_CAN_H__

#define FROM_STEERING (1)
#define FROM_BRAKE (2)
#define FROM_THROTTLE (3)
#define TO_STEERING (4)
#define TO_BRAKE (5)
#define TO_THROTTLE (6)

/*num FromVehicle {
  FROM_STEERING = 1,
  FROM_BRAKE = 2,
  FROM_THROTTLE = 3
};

enum ToVehicle {
  TO_STEERING = 4,
  TO_BRAKE = 5,
  TO_THROTTLE = 6,
};*/

//Initializers
void CAN0_Init(uint32_t baud);
void CAN1_Init(uint32_t baud);

void CAN_Init_MsgObj();

//Interrupt handlers
void CAN0_IntHdlr();
void CAN1_IntHdlr();

#endif //_MDAS_CAN_H
