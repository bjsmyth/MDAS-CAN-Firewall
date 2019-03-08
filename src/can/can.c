#include "can.h"

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/can.h"
#include "driverlib/pin_map.h"
//#include "inc/hw_gpio.h"
//#include "inc/hw_can.h"
#include "inc/hw_memmap.h"

void CAN0_Init(uint32_t baud) {
  //Enable peripheral clocks
  SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_CAN0) || !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));

  //Configure Port E GPIO for CAN0
  GPIOPinConfigure(GPIO_PE4_CAN0RX);
  GPIOPinConfigure(GPIO_PE5_CAN0TX);
  GPIOPinTypeCAN(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);

  //Configure and enable CAN0
  CANInit(CAN0_BASE);
  CANBitRateSet(CAN0_BASE, SysCtlClockGet(), baud);
  CANEnable(CAN0_BASE);

}
