#include <stdint.h>
#include <stdbool.h>

#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/can.h"
#include "SEGGER/SEGGER_RTT.h"

#include "can/can.h"

//Global variables
char testBuffer[1024];
volatile int testVal;

//Functions prototypes
void PortFInit(void);
void PLL_Init(void);
void SysTick_Init(void);
void SysTick_Wait1ms(unsigned long ms);


int main(void)
{
    PLL_Init();
    SysTick_Init();
    PortFInit();

    CAN0_Init(250000);
    CAN1_Init(500000);
    CAN_Init_MsgObj();

    IntMasterEnable();

    //SEGGER_RTT_Init();

    //testVal = SEGGER_RTT_ConfigUpBuffer(1, "MDAS_Gateway", testBuffer, sizeof(testBuffer), SEGGER_RTT_MODE_NO_BLOCK_SKIP);
    //SEGGER_RTT_WriteString(1, "SEGGER Real-Time-Terminal Sample\r\n\r\n");

    for(;;)
    {
      //SEGGER_RTT_printf(1,"%d\n", testVal);

      //GPIO_PORTF_DATA_R ^= 0x04;

    }
}

//Setup port F
void PortFInit(void)
{
    __attribute__((unused)) volatile uint32_t delay; //Supress unused variable warning with attibute
    SYSCTL_RCGC2_R |= 0x00000020;   //F clock
    delay = SYSCTL_RCGC2_R;         //delay
    GPIO_PORTF_LOCK_R = 0x4C4F434B; //Unlock Port F
    GPIO_PORTF_CR_R = 0x1F;         //Allow changes to PF4-0
    GPIO_PORTF_AMSEL_R = 0x00;      //Disable analog functions
    GPIO_PORTF_PCTL_R = 0x00000000; //GPIO clear bit PCTL
    GPIO_PORTF_DIR_R = 0x0E;        //PF2, PF1 output, PF4,PF0 input
    GPIO_PORTF_AFSEL_R = 0x00;      //Clear alt functions
    GPIO_PORTF_PUR_R |= 0x11;       //Enable pull-up resistor on PF4 and PF0
    GPIO_PORTF_DEN_R |= 0x1F;       //Enable digital pin PF4, PF0 and PF3-1
}

//Using the 400MHz PLL we get the bus frequency by dividing
//400MHz / (SYSDIV2+1). Therefore, 400MHz/(4+1) = 80 MHz bus frequency
#define SYSDIV2 4
//Gets clock from PLL
void PLL_Init(void)
{
    SYSCTL_RCC2_R |= SYSCTL_RCC2_USERCC2;   //Use RCC2 for advanced features
    SYSCTL_RCC2_R |= SYSCTL_RCC2_BYPASS2;   //Bypass PLL during initialization
    SYSCTL_RCC_R &= ~SYSCTL_RCC_XTAL_M;     //Clear XTAL field
    SYSCTL_RCC_R += SYSCTL_RCC_XTAL_16MHZ;  //Use 16Mhz crystal
    SYSCTL_RCC2_R &= ~SYSCTL_RCC2_OSCSRC2_M;//Clear oscillator source field
    SYSCTL_RCC2_R += SYSCTL_RCC2_OSCSRC2_MO;//Use main oscillator source field
    SYSCTL_RCC2_R &= ~SYSCTL_RCC2_PWRDN2;   //Activate PLL by clearing PWRDN

    //As explained above we use 400MHz PLL and specified SYSDIV2 for 80MHz clock
    SYSCTL_RCC2_R |= SYSCTL_RCC2_DIV400;    //Use 400MHz PLL
    //Clear system clock divider field then configure 80Mhz clock
    SYSCTL_RCC2_R = (SYSCTL_RCC2_R&~0x1FC00000) + (SYSDIV2<<22);
    //Wait for the PLL to lock by polling PLLLRIS
    while((SYSCTL_RIS_R&SYSCTL_RIS_PLLLRIS) == 0){};
    SYSCTL_RCC2_R &= ~SYSCTL_RCC2_BYPASS2;  //Enable PLL by clearing BYPASS
}

#undef SYSDIV2

void SysTick_Init(void)
{
    NVIC_ST_CTRL_R = 0;     //Disable SysTick during setup
    NVIC_ST_CURRENT_R = 0;  //Any write clears current
    //Enable SysTick with core clock, i.e. NVIC_ST_CTRL_R = 0x00000005
    NVIC_ST_CTRL_R = NVIC_ST_CTRL_ENABLE+NVIC_ST_CTRL_CLK_SRC;
}

//Uses SysTick to count down 1ms*(passed value)
void SysTick_Wait1ms(unsigned long ms)
{
    //Clock is set to 80MHz, each Systick takes 1/80MHz = 12.5ns
    //To get 1ms count down delay, take (1ms)/(12.5ns)
    unsigned long delay = 80000;
    unsigned long i;

    for(i = 0; i < ms; i++)
    {
        NVIC_ST_RELOAD_R = delay - 1;   //Reload value is the number of counts to wait
        NVIC_ST_CURRENT_R = 0;          //Clears current
        //Bit 16 of STCTRL is set to 1 if SysTick timer counts down to zero
        while((NVIC_ST_CTRL_R&0x00010000) == 0){}
    }
}
