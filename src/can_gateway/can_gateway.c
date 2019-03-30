#include "can_gateway.h"

//Global variables
static tCANMsgObject msgBlock[9];
static uint8_t msgData[8][8];
static uint32_t can0TxQueue = 0, can1TxQueue = 0;

static const uint32_t conCanIDs[3] = {CONTROLLER_BRAKE_ID, CONTROLLER_STEERING_ID, CONTROLLER_THROTTLE_ID};

//Local Functions
//Interrupt handlers
static void CAN0_IntHdlr();
static void CAN1_IntHdlr();



void CAN0_Init(uint32_t baud) {
  //Enable peripheral clocks
  SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_CAN0) ||
        !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));

  //Configure Port E GPIO for CAN0
  GPIOPinConfigure(GPIO_PE4_CAN0RX);
  GPIOPinConfigure(GPIO_PE5_CAN0TX);
  GPIOPinTypeCAN(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);

  //Configure CAN0
  CANInit(CAN0_BASE);
  CANBitRateSet(CAN0_BASE, SysCtlClockGet(), baud);

  //Configure CAN0 Interrupt
  CANIntRegister(CAN0_BASE, &CAN0_IntHdlr);
  CANIntEnable(CAN0_BASE, CAN_INT_MASTER);
  IntPrioritySet(INT_CAN0, 1); //Controller to Vehicle must be higher priority
  IntEnable(INT_CAN0);
  CANEnable(CAN0_BASE);
}

void CAN1_Init(uint32_t baud) {
  //Enable peripheral clocks
  SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN1);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_CAN1) ||
        !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));

  //Configure Port A GPIO for CAN1
  GPIOPinConfigure(GPIO_PA0_CAN1RX);
  GPIOPinConfigure(GPIO_PA1_CAN1TX);
  GPIOPinTypeCAN(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  //Configure and enable CAN1
  CANInit(CAN1_BASE);
  CANBitRateSet(CAN1_BASE, SysCtlClockGet(), baud);

  //Configure CAN1 Interrupt
  CANIntRegister(CAN1_BASE, &CAN1_IntHdlr);
  CANIntEnable(CAN1_BASE, CAN_INT_MASTER);
  IntPrioritySet(INT_CAN1, 1); //Controller to Vehicle must be higher priority
  IntEnable(INT_CAN1);
  CANEnable(CAN1_BASE);
}

void CAN_Init_MsgObj() {
  //Setup standard message information
  for(int i = 1; i < 9; i++) {
    msgBlock[i].ui32MsgIDMask = 0x1FFFFFFF;
    msgBlock[i].ui32MsgLen = 8;
    msgBlock[i].ui32Flags = MSG_OBJ_USE_ID_FILTER | MSG_OBJ_EXTENDED_ID;
    msgBlock[i].pui8MsgData = msgData[i-1];
  }

  //Setup message interrupt flags
  msgBlock[4].ui32Flags |= MSG_OBJ_RX_INT_ENABLE;
  msgBlock[8].ui32Flags |= MSG_OBJ_TX_INT_ENABLE;
  for(int i = 5; i < 8; i++) {
    msgBlock[i].ui32Flags |= MSG_OBJ_RX_INT_ENABLE;
    msgBlock[i-4].ui32Flags |= MSG_OBJ_TX_INT_ENABLE;
  }

  //Setup message IDs
  msgBlock[4].ui32MsgID = VEHICLE_FEEDBACK_ID;
  msgBlock[8].ui32MsgID = VEHICLE_FEEDBACK_ID;
  for(int i = 1; i < 4; i++) {
    msgBlock[i].ui32MsgID = conCanIDs[i-1];
    msgBlock[i+4].ui32MsgID = conCanIDs[i-1];
  }

  //Setup RX messages with CAN module
  CANMessageSet(CAN0_BASE, 4, &msgBlock[4], MSG_OBJ_TYPE_RX);
  for(int i = 5; i < 9; i++) {
    CANMessageSet(CAN1_BASE, i-4, &msgBlock[i], MSG_OBJ_TYPE_RX);
  }
}

static void CAN0_IntHdlr() {
  register uint32_t status = CANIntStatus(CAN0_BASE, CAN_INT_STS_CAUSE);

  register uint32_t vehicle_can0, controller_can1;

  switch (status) {
    case CAN_INT_INTID_STATUS:
      status = CANStatusGet(CAN0_BASE, CAN_STS_CONTROL);
      g_can0ErrorFlags |= status;
      CANIntClear(CAN0_BASE, CAN_INT_INTID_STATUS);
      break;
    case CONTROLLER_STEERING:
    case CONTROLLER_THROTTLE:
    case CONTROLLER_BRAKE:
      can0TxQueue--;
      if(can0TxQueue == UINT32_MAX) {
        can0TxQueue = 0; //Reset underflow
      }
      if(can0TxQueue == 0) {
        GPIO_PORTF_DATA_R &= ~LED_B;
      }

      CANIntClear(CAN0_BASE, status);
      break;
    case VEHICLE_FEEDBACK:
      GPIO_PORTF_DATA_R |= LED_G;
      vehicle_can0 = status;
      controller_can1 = vehicle_can0 + 4;

      CANMessageGet(CAN0_BASE, status, &msgBlock[vehicle_can0], true);

      IntMasterDisable();
      memcpy(msgBlock[controller_can1].pui8MsgData, msgBlock[vehicle_can0].pui8MsgData, msgBlock[vehicle_can0].ui32MsgLen);
      msgBlock[controller_can1].ui32MsgLen = msgBlock[vehicle_can0].ui32MsgLen;

      CANMessageSet(CAN1_BASE, status, &msgBlock[controller_can1], MSG_OBJ_TYPE_TX);
      IntMasterEnable();

      CANIntClear(CAN0_BASE, status);

      can1TxQueue++;
      break;
  }
}

static void CAN1_IntHdlr() {
  register uint32_t status = CANIntStatus(CAN1_BASE, CAN_INT_STS_CAUSE);

  register uint32_t vehicle_can0, controller_can1;

  switch (status) {
    case CAN_INT_INTID_STATUS:
      status = CANStatusGet(CAN1_BASE, CAN_STS_CONTROL);
      g_can1ErrorFlags |= status;
      CANIntClear(CAN1_BASE, CAN_INT_INTID_STATUS);
      break;
    case CONTROLLER_STEERING:
    case CONTROLLER_THROTTLE:
    case CONTROLLER_BRAKE:
      GPIO_PORTF_DATA_R |= LED_B;

      vehicle_can0 = status;
      controller_can1 = vehicle_can0 + 4;

      CANMessageGet(CAN1_BASE, status, &msgBlock[controller_can1], true);

      IntMasterDisable();
      memcpy(msgBlock[vehicle_can0].pui8MsgData, msgBlock[controller_can1].pui8MsgData, msgBlock[controller_can1].ui32MsgLen);
      msgBlock[vehicle_can0].ui32MsgLen = msgBlock[controller_can1].ui32MsgLen;

      CANMessageSet(CAN0_BASE, status, &msgBlock[vehicle_can0], MSG_OBJ_TYPE_TX);
      IntMasterEnable();

      CANIntClear(CAN1_BASE, status);

      can0TxQueue++;
      break;
    case VEHICLE_FEEDBACK:
      can1TxQueue--;
      if(can1TxQueue == UINT32_MAX) {
        can1TxQueue = 0; //Reset underflow
      }
      if(can1TxQueue == 0) {
        GPIO_PORTF_DATA_R &= ~LED_G;
      }

      CANIntClear(CAN1_BASE, status);
      break;
  }
}
