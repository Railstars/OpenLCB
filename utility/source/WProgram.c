// Copyright (c) 2011 Per Eklund, D.E. Goodman-Wilson
// Arduino stand-in
#include <stdint.h>
#include "LPC17xx.h"
#include "lpc17xx_rit.h"

#define PCRIT 16
//the above is from p. 63, LPC167x User's Manual
#define TIME_INTERVAL 	120000

volatile uint32_t _count=0;
volatile uint32_t _millisec=0;

/************************** PRIVATE FUNCTION *************************/


/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief		RIT interrupt handler sub-routine
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if defined (__cplusplus)
extern "C" {
#endif
void RIT_IRQHandler(void)
{
    //LPC_GPIO0->FIOPIN ^= (1 << 22);
	++ _millisec;
	LPC_RIT->RICTRL |= RIT_CTRL_INTEN; //clear interrupt flag by writing '1'
	//RIT_GetIntStatus(LPC_RIT); //call this to clear interrupt flag
}


void InitTimers(void)
{
    //bit of an update for CMSIS V2.0 DEGW 20 August 2011
    RIT_CMP_VAL  value;
    value.CMPVAL = TIME_INTERVAL;
    value.COUNTVAL = 0x00000000;
    value.MASKVAL = 0x00000000;

	RIT_Init(LPC_RIT);
	/* Configure time_interval for RIT
	 * In this case: time_interval = 1 ms
	 * So, RIT will generate interrupt each 1s
	 */
	RIT_TimerConfig(LPC_RIT, &value);
	RIT_TimerClearCmd(LPC_RIT, ENABLE); //enable automatic clearing of counter at compare match.
	//RIT_TimerConfig(LPC_RIT, TIME_INTERVAL);
	//RIT_Cmd(LPC_RIT, ENABLE); //shouldn't be needed; timer is enabled by default
	NVIC_EnableIRQ(RIT_IRQn);

}

void init(void)
{
  InitTimers();
}

uint32_t millis(void)
{
	return _millisec;
}

void delay(uint32_t ms)
{
    uint32_t currentTime;

    currentTime = _millisec;       // read current tick counter
    // Now loop until required number of ticks passes.
    while ((_millisec - currentTime) < ms);
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


#if defined (__cplusplus)
}
#endif
