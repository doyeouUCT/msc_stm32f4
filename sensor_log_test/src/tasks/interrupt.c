/*
 * interrupt.c
 *
 *  Created on: Mar 5, 2018
 *      Author: Do Yeou Ku
 */

#include "interrupt.h"

/*-----------------------------------------------------------------------
 Module Private Functions and Variables
 ------------------------------------------------------------------------*/
static
void exti_on (void)
{
	/* EXTI GPIO Configuration --------------------------------------------------*/
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable clock for SYSCFG */
    RCC_APBPeriphClockCmd_EXTI(RCC_APBPeriph_EXTI, ENABLE);
    /* Enable clock for GPIOx */
    RCC_AHBPeriphClockCmd_EXTI_GPIO(RCC_AHBPeriph_EXTI_GPIO, ENABLE);

    /* Set pin as input */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

    /* EXTI Accelerometer pin configuration */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_EXTI_ACCEL;
    GPIO_Init(GPIO_EXTI, &GPIO_InitStructure);

    /* EXTI Gyroscope pin configuration */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_EXTI_GYRO;
    GPIO_Init(GPIO_EXTI, &GPIO_InitStructure);

    /* EXTI Magnetometer pin configuration */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_EXTI_MAG;
    GPIO_Init(GPIO_EXTI, &GPIO_InitStructure);

    /* EXTI Barometer pin configuration */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_EXTI_BARO;
    GPIO_Init(GPIO_EXTI, &GPIO_InitStructure);

    /* EXTI PPS pin configuration */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_EXTI_PPS;
    GPIO_Init(GPIO_EXTI, &GPIO_InitStructure);

    /* SYSCFG EXTI_Line configuration*/
    GPIO_EXTILine_Config(GPIO_Pin_EXTI_Src, GPIO_Pin_EXTI_ACCEL_Src);
    GPIO_EXTILine_Config(GPIO_Pin_EXTI_Src, GPIO_Pin_EXTI_GYRO_Src);
    GPIO_EXTILine_Config(GPIO_Pin_EXTI_Src, GPIO_Pin_EXTI_MAG_Src);
    GPIO_EXTILine_Config(GPIO_Pin_EXTI_Src, GPIO_Pin_EXTI_BARO_Src);
    GPIO_EXTILine_Config(GPIO_Pin_EXTI_Src, GPIO_Pin_EXTI_PPS_Src);

    /* EXTI Configuration --------------------------------------------------*/
    EXTI_InitTypeDef EXTI_InitStructure;

    EXTI_InitStructure.EXTI_Line = EXTILine_ACCEL|EXTILine_GYRO|EXTILine_MAG|EXTILine_BARO|EXTILine_PPS;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&EXTI_InitStructure);

    /* EXTI NVIC Configuration --------------------------------------------------*/
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Accelerometer */
    NVIC_InitStructure.NVIC_IRQChannel = EXTILine_ACCEL_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EXTILine_Priority;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = EXTILine_ACCEL_Priority;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Gyroscope */
    NVIC_InitStructure.NVIC_IRQChannel = EXTILine_GYRO_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EXTILine_Priority;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = EXTILine_GYRO_Priority;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Magnetometer */
    NVIC_InitStructure.NVIC_IRQChannel = EXTILine_MAG_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EXTILine_Priority;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = EXTILine_MAG_Priority;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Barometer */
    NVIC_InitStructure.NVIC_IRQChannel = EXTILine_BARO_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EXTILine_Priority;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = EXTILine_BARO_Priority;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* PPS */
    NVIC_InitStructure.NVIC_IRQChannel = EXTILine_PPS_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EXTILine_Priority;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = EXTILine_PPS_Priority;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*-----------------------------------------------------------------------
 Message Frame functions
 ------------------------------------------------------------------------*/

static
void add_prefix (SENSOR sensor)
{
	time_stamp.counterIII = TIM_GetCounter(TIMER_PPS);
	(sensor_buffer[sensor_crnt_buffer])[sensor_crnt_pos++] = '"';
	switch(sensor){
	case(ACCEL)	:	(sensor_buffer[sensor_crnt_buffer])[sensor_crnt_pos++] = BMX055_ACC_ID;	break;
	case(GYRO)	:	(sensor_buffer[sensor_crnt_buffer])[sensor_crnt_pos++] = BMX055_GYRO_ID;	break;
	case(MAG)	:	(sensor_buffer[sensor_crnt_buffer])[sensor_crnt_pos++] = BMX055_MAG_ID;	break;
	default		:	;}
	(sensor_buffer[sensor_crnt_buffer])[sensor_crnt_pos++] = '"';
	for (int i = 0; i < 4; i++){
		(sensor_buffer[sensor_crnt_buffer])[sensor_crnt_pos++] = (uint8_t)((time_stamp.counterI & ((uint32_t)0xFF<<(24-8*i)))>>(24-8*i));
	}
	for (int i = 0; i < 2; i++){
		(sensor_buffer[sensor_crnt_buffer])[sensor_crnt_pos++] = (uint8_t)((time_stamp.counterII & ((uint32_t)0xFF<<(8-8*i)))>>(8-8*i));
	}
	for (int i = 0; i < 2; i++){
		(sensor_buffer[sensor_crnt_buffer])[sensor_crnt_pos++] = (uint8_t)((time_stamp.counterIII & ((uint32_t)0xFF<<(8-8*i)))>>(8-8*i));
	}
}

static
void add_suffix (void)
{
	(sensor_buffer[sensor_crnt_buffer])[sensor_crnt_pos++] = '\n';
}

static
void add_error_message (void)
{
	(sensor_buffer[sensor_crnt_buffer])[sensor_crnt_pos++] = 'e';
	(sensor_buffer[sensor_crnt_buffer])[sensor_crnt_pos++] = 'r';
	(sensor_buffer[sensor_crnt_buffer])[sensor_crnt_pos++] = 'r';
	(sensor_buffer[sensor_crnt_buffer])[sensor_crnt_pos++] = 'o';
	(sensor_buffer[sensor_crnt_buffer])[sensor_crnt_pos++] = 'r';
	(sensor_buffer[sensor_crnt_buffer])[sensor_crnt_pos++] = '!';
}

static
void check_overflow (void)
{
	if ((sensor_crnt_pos + MAX_DATA_LENGTH ) > SENSOR_LOG_BUFFER_SIZE){
		sensor_bytes_to_log 	=	sensor_crnt_pos;
		sensor_address_to_log	=	sensor_buffer[sensor_crnt_buffer];
		sensor_crnt_pos			=	0;
		sensor_crnt_buffer		=	(sensor_crnt_buffer + 1)%3;
		sensor_log_on			=	SENSOR_LOG_ON;
	}
}
/*-----------------------------------------------------------------------
 Update triggered by PPS
 ------------------------------------------------------------------------*/
static
void update_by_pps (void)
{
	TIM_SetCounter(TIMER_PPS, 0);
	time_stamp.counterI		+=	1;
	time_stamp.counterII	=	0;
}
/*-----------------------------------------------------------------------

 Public Functions

 ------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/* Initialize External Interrupt handlers                                */
/*-----------------------------------------------------------------------*/

void exti_initialize (void){
	timer_enabled = 0;
	timestamp_initialize();
	exti_on();
}

/*-----------------------------------------------------------------------*/
/* External Interrupt handlers                                           */
/*-----------------------------------------------------------------------*/
void EXTILine_ACCEL_IRQHandler(void){
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTILine_ACCEL) != RESET) {
    	STM_EVAL_LEDOn	(LED4);
    	add_prefix		(ACCEL);
    	if(accel_read(&((sensor_buffer[sensor_crnt_buffer])[sensor_crnt_pos]), ACCEL_DATA_LENGTH) != SENS_OK)	add_error_message();
    	else	sensor_crnt_pos += ACCEL_DATA_LENGTH;
    	//while(accel_read(&((sensor_buffer[sensor_crnt_buffer])[sensor_crnt_pos]), ACCEL_DATA_LENGTH) != SENS_OK);
    	//sensor_crnt_pos += ACCEL_DATA_LENGTH;
    	add_suffix		();
    	check_overflow	();
    	STM_EVAL_LEDOff	(LED4);
    	/* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTILine_ACCEL);
    }
}

void EXTILine_GYRO_IRQHandler(void){
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTILine_GYRO) != RESET) {
    	STM_EVAL_LEDOn	(LED4);
    	add_prefix		(GYRO);
    	if(gyro_read(&((sensor_buffer[sensor_crnt_buffer])[sensor_crnt_pos]), GYRO_DATA_LENGTH) != SENS_OK)	add_error_message();
    	else	sensor_crnt_pos += GYRO_DATA_LENGTH;
//    	while(gyro_read(&((sensor_buffer[sensor_crnt_buffer])[sensor_crnt_pos]), GYRO_DATA_LENGTH) != SENS_OK);
//    	sensor_crnt_pos += GYRO_DATA_LENGTH;
    	add_suffix		();
    	check_overflow	();
    	STM_EVAL_LEDOff	(LED4);
    	/* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTILine_GYRO);
    }
}

void EXTILine_MAG_IRQHandler(void){
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTILine_MAG) != RESET) {

        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTILine_MAG);
    }
}

void EXTILine_BARO_IRQHandler(void){
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTILine_BARO) != RESET) {
    	file_flag.log_enabled = 0;
        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTILine_BARO);
    }
}

void EXTILine_PPS_IRQHandler(void){
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTILine_PPS) != RESET) {
    	if(timer_enabled == 0) {
    		timestamp_enable();
    		timer_enabled = 1;
    	}
    	else update_by_pps();
        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTILine_PPS);
    }
}
