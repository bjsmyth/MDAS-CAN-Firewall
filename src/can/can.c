#include "can.h"

//Global variables
volatile uint32_t CAN0ErrorFlags = 0;

static tCANMsgObject msgBlock[7];
static uint8_t msgData[7][8];

static const uint32_t canIDs[6] = {0x1ADB0000, 0x011EEEEE, 0x013EEEEE, 0x1ADB0000, 0x011EEEEE, 0x013EEEEE};

void CAN0_Init(uint32_t baud) {
  //Enable peripheral clocks
  SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_CAN0) || !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));

  //Configure Port E GPIO for CAN0
  GPIOPinConfigure(GPIO_PE4_CAN0RX);
  GPIOPinConfigure(GPIO_PE5_CAN0TX);
  GPIOPinTypeCAN(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);

  //Configure CAN0
  CANInit(CAN0_BASE);
  CANBitRateSet(CAN0_BASE, SysCtlClockGet(), baud);

  //Configure CAN0 Interrupt
  CANIntRegister(CAN0_BASE, &CAN0_IntHdlr);
  CANIntEnable(CAN0_BASE, CAN_INT_MASTER | CAN_INT_STATUS);
  IntEnable(INT_CAN0);
  CANEnable(CAN0_BASE);
}

void CAN1_Init(uint32_t baud) {
  //Enable peripheral clocks
  SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN1);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_CAN1) || !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));

  //Configure Port A GPIO for CAN1
  GPIOPinConfigure(GPIO_PA0_CAN1RX);
  GPIOPinConfigure(GPIO_PA1_CAN1TX);
  GPIOPinTypeCAN(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  //Configure and enable CAN1
  CANInit(CAN1_BASE);
  CANBitRateSet(CAN1_BASE, SysCtlClockGet(), baud);

  //Configure CAN1 Interrupt
  CANIntRegister(CAN1_BASE, &CAN1_IntHdlr);
  CANIntEnable(CAN1_BASE, CAN_INT_MASTER | CAN_INT_STATUS);
  IntEnable(INT_CAN1);
  CANEnable(CAN1_BASE);
}

void CAN_Init_MsgObj() {
  //uint32_t canIDs[6] = {0x01, 0x2, 0x03, 0xDEC, 0x5, 0x6};
    for(int i = 1; i < 7; i++) {
      msgBlock[i].ui32MsgID = canIDs[i-1];
      msgBlock[i].ui32MsgIDMask = 0x1FFFFFFF;
      msgBlock[i].ui32MsgLen = 8;
      msgBlock[i].ui32Flags = MSG_OBJ_USE_ID_FILTER | MSG_OBJ_EXTENDED_ID;
      msgBlock[i].pui8MsgData = msgData[i];
    }

    msgBlock[1].ui32MsgID = canIDs[0];

    for(int i = 1; i < 4; i++) {
      msgBlock[i].ui32Flags |= MSG_OBJ_RX_INT_ENABLE;
    }

    for(int i = 1; i < 4; i++) {
      CANMessageSet(CAN0_BASE, i, &msgBlock[i], MSG_OBJ_TYPE_RX);
      CANMessageSet(CAN1_BASE, i+3, &msgBlock[i+3], MSG_OBJ_TYPE_RX);
    }
}

void CAN0_IntHdlr() {

  uint32_t status = CANIntStatus(CAN0_BASE, CAN_INT_STS_CAUSE);

  uint32_t vehicle, controller;

  switch (status) {
    case CAN_INT_INTID_STATUS:
      status = CANStatusGet(CAN0_BASE, CAN_STS_CONTROL);
      CAN0ErrorFlags |= status;
      CANIntClear(CAN0_BASE, CAN_INT_INTID_STATUS);
      break;
    case VEHICLE_STEERING:
    case VEHICLE_THROTTLE:
    case VEHICLE_BRAKE:
      GPIO_PORTF_DATA_R ^= 0x04;
      vehicle = status;
      controller = status + 3;

      CANMessageGet(CAN0_BASE, vehicle, &msgBlock[vehicle], true);

      IntMasterDisable();
      memcpy(msgBlock[controller].pui8MsgData, msgBlock[vehicle].pui8MsgData, msgBlock[vehicle].ui32MsgLen);
      msgBlock[controller].ui32MsgLen = msgBlock[vehicle].ui32MsgLen;
      IntMasterEnable();

      CANMessageSet(CAN1_BASE, controller, &msgBlock[controller], MSG_OBJ_TYPE_TX);

      CANIntClear(CAN0_BASE, vehicle);

      GPIO_PORTF_DATA_R ^= 0x04;
      break;
    case CONTROLLER_STEERING:
    case CONTROLLER_THROTTLE:
    case CONTROLLER_BRAKE:
      //Clear interrupt, may be used later
      CANIntClear(CAN0_BASE, status);
      break;
  }

}

void CAN1_IntHdlr() {
  GPIO_PORTF_DATA_R ^= 0x08;

  uint32_t status = CANIntStatus(CAN1_BASE, CAN_INT_STS_CAUSE);

  uint32_t vehicle, controller;

  switch (status) {
    case CAN_INT_INTID_STATUS:
      status = CANStatusGet(CAN1_BASE, CAN_STS_CONTROL);
      CAN0ErrorFlags |= status;
      CANIntClear(CAN1_BASE, CAN_INT_INTID_STATUS);
      break;
    case VEHICLE_STEERING:
    case VEHICLE_THROTTLE:
    case VEHICLE_BRAKE:
      //Clear interrupt, may be used later
      CANIntClear(CAN1_BASE, status);
      break;
    case CONTROLLER_STEERING:
    case CONTROLLER_THROTTLE:
    case CONTROLLER_BRAKE:
      controller = status;
      vehicle = status - 3;

      CANMessageGet(CAN1_BASE, controller, &msgBlock[controller], true);

      IntMasterDisable();
      memcpy(msgBlock[vehicle].pui8MsgData, msgBlock[controller].pui8MsgData, msgBlock[controller].ui32MsgLen);
      msgBlock[vehicle].ui32MsgLen = msgBlock[controller].ui32MsgLen;
      IntMasterEnable();

      CANMessageSet(CAN0_BASE, vehicle, &msgBlock[vehicle], MSG_OBJ_TYPE_TX);

      CANIntClear(CAN1_BASE, controller);
      break;

  }
  GPIO_PORTF_DATA_R ^= 0x08;

}
