#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/can.h"

#include "can_gateway/can_gateway.h"
#include "globals_macros/globals_macros.h"

//Global variables

//Functions prototypes
void PortDInit(void);
void PortEInit(void);
void PortFInit(void);
void PLL_Init(void);


int main(void)
{
    PLL_Init();
    PortFInit();
    PortDInit();
    PortEInit();

    //Read pins 1 and 2 and invert due to hardware layout
    uint8_t canSwitch = ((~GPIO_PORTE_DATA_R) >> 1) & 0x03;

    uint32_t can1Baud = 250000; //Default CAN1 bitrate
    if(canSwitch == 0) {
      can1Baud = 250000; //250k switch config 00
    }
    else if(canSwitch == 1) {
      can1Baud = 500000; //500k switch config 01
    }
    else if(canSwitch == 2) {
      can1Baud = 125000; //125k switch config 10
    }
    else if(canSwitch == 3) {
      can1Baud = 1000000; //1M switch config 11
    }

    CAN0_Init(250000);
    CAN1_Init(can1Baud);
    CAN_Init_MsgObj();

    IntMasterEnable();

    uint32_t can0Status, can0StatusPrev, can1Status, can1StatusPrev;

    for(;;)
    {
        can0Status = CANStatusGet(CAN0_BASE, CAN_STS_CONTROL);
        can1Status = CANStatusGet(CAN1_BASE, CAN_STS_CONTROL);

        if((can0Status & CAN_STATUS_EPASS) == CAN_STATUS_EPASS ||
           (can1Status & CAN_STATUS_EPASS) == CAN_STATUS_EPASS) {
          GPIO_PORTF_DATA_R |= LED_R;
        }
        else {
          GPIO_PORTF_DATA_R &= ~LED_R;
        }

        // If bus error disappears turn off tx LED and reset queue counter
        if((can0Status & CAN_STATUS_EPASS) != CAN_STATUS_EPASS &&
           (can0StatusPrev & CAN_STATUS_EPASS) == CAN_STATUS_EPASS) {
          GPIO_PORTF_DATA_R &= ~LED_B;
          g_can0TxQueue = 0;
        }
        if((can1Status & CAN_STATUS_EPASS) != CAN_STATUS_EPASS &&
           (can1StatusPrev & CAN_STATUS_EPASS) == CAN_STATUS_EPASS) {
          GPIO_PORTF_DATA_R &= ~LED_G;
          g_can1TxQueue = 0;
        }

        can0StatusPrev = can0Status;
        can1StatusPrev = can1Status;
    }
}

//Setup port F
void PortFInit(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, LED_R | LED_G | LED_B);
}

//Setup port D
void PortDInit(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD));
  GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_6 | GPIO_PIN_7);
}

//Setup port E
void PortEInit(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));
  GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2);
}

//Gets clock from PLL
void PLL_Init(void)
{
  SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL |
                 SYSCTL_OSC_MAIN | SYSCTL_RCC_XTAL_16MHZ);
}
