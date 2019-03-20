#include "can.h"

//Global variables
volatile uint32_t CAN0ErrorFlags = 0;

static tCANMsgObject msgBlock[7];
static uint8_t msgData[7][8];

static const uint32_t canIDs[6] = {0x01, 0x2, 0x03, 0xDEC, 0x5, 0x6};

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
  CANEnable(CAN1_BASE);


}

void CAN_Init_MsgObj() {
  //uint32_t canIDs[6] = {0x01, 0x2, 0x03, 0xDEC, 0x5, 0x6};
    for(int i = 1; i < 7; i++) {
      msgBlock[i].ui32MsgID = canIDs[i-1];
      msgBlock[i].ui32MsgIDMask = ~(canIDs[i-1]);
      msgBlock[i].ui32MsgLen = 8;
      msgBlock[i].ui32Flags = MSG_OBJ_USE_ID_FILTER;
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

  /*if(status == CAN_INT_INTID_STATUS) {
    CANIntClear(CAN0_BASE, CAN_INT_INTID_STATUS);
    status = CANStatusGet(CAN0_BASE, CAN_STS_CONTROL);

    CAN0ErrorFlags |= status;
  }
  else if(status == FROM_STEERING) {
    CANIntClear(CAN0_BASE, FROM_STEERING);

    //Get message from CAN0 registers
    CANMessageGet(CAN0_BASE, FROM_STEERING, &msgBlock[FROM_STEERING], true);

    //Disable interrupts, copy data from CAN0 to CAN1, reenable interrupts
    IntMasterDisable();
    memcpy(msgBlock[TO_STEERING].pui8MsgData, msgBlock[FROM_STEERING].pui8MsgData, msgBlock[FROM_STEERING].ui32MsgLen);
    msgBlock[TO_STEERING].ui32MsgLen = msgBlock[FROM_STEERING].ui32MsgLen;
    IntMasterEnable();

    CANMessageSet(CAN1_BASE, TO_STEERING, &msgBlock[TO_STEERING], MSG_OBJ_TYPE_TX);
  }
  else if(status == TO_STEERING) {
    CANIntClear(CAN0_BASE, TO_STEERING);
  }*/

  uint32_t from, to;

  switch (status) {
    case CAN_INT_INTID_STATUS:
      status = CANStatusGet(CAN0_BASE, CAN_STS_CONTROL);
      CAN0ErrorFlags |= status;
      CANIntClear(CAN0_BASE, CAN_INT_INTID_STATUS);
      break;
    case FROM_STEERING:
    case FROM_THROTTLE:
    case FROM_BRAKE:
      from = status;
      to = status + 3;

      CANMessageGet(CAN0_BASE, from, &msgBlock[from], true);

      IntMasterDisable();
      memcpy(msgBlock[to].pui8MsgData, msgBlock[from].pui8MsgData, msgBlock[from].ui32MsgLen);
      msgBlock[to].ui32MsgLen = msgBlock[from].ui32MsgLen;
      IntMasterEnable();

      CANMessageSet(CAN1_BASE, to, &msgBlock[to], MSG_OBJ_TYPE_TX);

      CANIntClear(CAN0_BASE, from);
      break;
    case TO_STEERING:
    case TO_THROTTLE:
    case TO_BRAKE:
      //Clear interrupt, may be used later
      CANIntClear(CAN0_BASE, status);
      break;
  }
}
