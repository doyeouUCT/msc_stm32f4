/*
******************************************************************************
File:		main.c
Info:		Generated by Atollic TrueSTUDIO(R) 9.0.0   2018-03-12
Author:		Ku, Do Yeou
******************************************************************************
*/

#include "main.h"

int main(void)
{
	/* Initialize LEDs */
	STM_EVAL_LEDInit(LED3);	STM_EVAL_LEDInit(LED4);	STM_EVAL_LEDInit(LED5);	STM_EVAL_LEDInit(LED6);

	/* Turn off LEDs */
	STM_EVAL_LEDOff(LED3);	STM_EVAL_LEDOff(LED4);	STM_EVAL_LEDOff(LED5);	STM_EVAL_LEDOff(LED6);

	gnss_initialize();
	STM_EVAL_LEDOn(LED3);

	/* Infinite loop */
	while (1)
	{

	}
}