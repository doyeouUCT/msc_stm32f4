/*
 * timeio.c
 *
 *  Created on: Mar 20, 2018
 *      Author: Do Yeou Ku
 */

#include "tasks/timeio.h"

/*-----------------------------------------------------------------------
 Module Private Functions and Variables
 ------------------------------------------------------------------------*/
static
void timestamp_on (void)
{
	/* Timer Configuration */
	RCC_APBPeriphClockCmd_PPS(RCC_APBPeriph_PPS, ENABLE);
	TIM_TimeBaseInitTypeDef TIM_InitStructure;
	TIM_InitStructure.TIM_Prescaler = TIMER_PPS_PSC;
	TIM_InitStructure.TIM_Period = TIMER_PPS_ARR;
	TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIMER_PPS, &TIM_InitStructure);
	// Update Event (Hz) = timer_clock / ((TIM_Prescaler + 1) * (TIM_Period + 1)
	// 82 MHz
	/* Interrupt Enable */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIMER_PPS_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIMER_PPS_Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = TIMER_PPS_Priority;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	TIM_ITConfig(TIMER_PPS, TIM_IT_Update, ENABLE);
}

/*-----------------------------------------------------------------------
 Update triggered by TIMER
 ------------------------------------------------------------------------*/
static
void update_by_timer (void)
{
	time_stamp.counterII	+=	1;
	time_stamp.counterIII	=	0;
}

/*-----------------------------------------------------------------------

 Public Functions

 ------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/* Enable Timer			                                                 */
/*-----------------------------------------------------------------------*/
void timestamp_enable (void)
{
	/* Timer Enable */
	TIM_Cmd(TIMER_PPS, ENABLE);
}

/*-----------------------------------------------------------------------*/
/* Initialize Timer		                                                 */
/*-----------------------------------------------------------------------*/
void timestamp_initialize (void)
{
	time_stamp.counterI		= 0;
	time_stamp.counterII	= 0;
	time_stamp.counterIII	= 0;
	timestamp_on();
}

/*-----------------------------------------------------------------------*/
/* Timer Interrupt handler	                                             */
/*-----------------------------------------------------------------------*/
void TIMER_PPS_IRQHandler (void)
{
    // Checks whether the TIM2 interrupt has occurred or not
    if (TIM_GetITStatus(TIMER_PPS, TIM_IT_Update))
    {
    	update_by_timer();
        // Clears the TIM2 interrupt pending bit
        TIM_ClearITPendingBit(TIMER_PPS, TIM_IT_Update);
    }
}
