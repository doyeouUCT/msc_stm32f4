/*
 * main.h
 *
 *  Created on: Mar 5, 2018
 *      Author: Do Yeou Ku
 */

#ifndef MAIN_H_
#define MAIN_H_

//#define testing

/* Standard includes. */
#include "stm32f4xx.h"
#include "stm32f4_discovery.h"

/* Task includes. */
#include "tasks/interrupt.h"
//#include "tasks/sensorio.h"
//#include "tasks/buffer.h"
//#include "fatfs/ff.h"

/* Private macro */
/* Private variables */
/* Private function prototypes */
/* Private functions */

/*
 * Callback used by stm32f4_discovery_audio_codec.c & stm324xg_eval_audio_codec.c
 * Refer to stm32f4_discovery_audio_codec.h for more info.
 */
void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size){ return; }
uint16_t EVAL_AUDIO_GetSampleCallBack(void){ return -1; }

#ifdef testing

/* SPI connection pin defined. */
#define RCC_AHBPeriphClockCmd_SPI_BOSCH_CS		RCC_AHB1PeriphClockCmd
#define RCC_AHBPeriph_GPIO_BOSCH_CS   			RCC_AHB1Periph_GPIOA
#define GPIO_BOSCH_CS	              			GPIOA
#define GPIO_Pin_ACC_CS              			GPIO_Pin_8
#define GPIO_Pin_GYRO_CS              			GPIO_Pin_10
#define GPIO_Pin_MAG_CS              			GPIO_Pin_11
#define GPIO_Pin_BARO_CS              			GPIO_Pin_11

#define RCC_APBPeriphClockCmd_SPI_BOSCH			RCC_APB2PeriphClockCmd
#define RCC_APBPeriph_SPI_BOSCH					RCC_APB2Periph_SPI1
#define SPI_BOSCH                   			SPI1
#define GPIO_SPI_BOSCH              			GPIOA
#define GPIO_Pin_SPI_BOSCH_SCK      			GPIO_Pin_5
#define GPIO_Pin_SPI_BOSCH_MISO     			GPIO_Pin_6
#define GPIO_Pin_SPI_BOSCH_MOSI     			GPIO_Pin_7

#define GPIO_Pin_SPI_BOSCH_SCK_Src				GPIO_PinSource5
#define GPIO_Pin_SPI_BOSCH_MISO_Src				GPIO_PinSource6
#define GPIO_Pin_SPI_BOSCH_MOSI_Src				GPIO_PinSource7

#define GPIO_AF_SPI_BOSCH						GPIO_AF_SPI1

/* SPI full-speed : 72MHz/4 TODO: check*/
#define SPI_BaudRatePrescaler_SPI_BOSCH			SPI_BaudRatePrescaler_8

/* Card-Select control defined. (Platform dependent) */
#define ACC_SELECT()        GPIO_ResetBits(GPIO_BOSCH_CS, GPIO_Pin_ACC_CS)	/* ACC CS = L */
#define ACC_DESELECT()      GPIO_SetBits(GPIO_BOSCH_CS, GPIO_Pin_ACC_CS)		/* ACC CS = H */
#define GYRO_SELECT()		GPIO_ResetBits(GPIO_BOSCH_CS, GPIO_Pin_GYRO_CS)	/* GYRO CS = L */
#define GYRO_DESELECT()		GPIO_SetBits(GPIO_BOSCH_CS, GPIO_Pin_GYRO_CS)		/* GYRO CS = H */
#define MAG_SELECT()		GPIO_ResetBits(GPIO_BOSCH_CS, GPIO_Pin_MAG_CS)	/* MAG CS = L */
#define MAG_DESELECT()		GPIO_SetBits(GPIO_BOSCH_CS, GPIO_Pin_MAG_CS)		/* MAG CS = H */
#define BARO_SELECT()		GPIO_ResetBits(GPIO_BOSCH_CS, GPIO_Pin_BARO_CS)	/* BARO CS = L */
#define BARO_DESELECT()		GPIO_SetBits(GPIO_BOSCH_CS, GPIO_Pin_BARO_CS)		/* BARO CS = H */

#else
#include "tasks/sensorio.h"
#endif


#endif /* MAIN_H_ */
