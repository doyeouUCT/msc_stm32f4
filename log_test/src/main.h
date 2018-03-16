/*
 * main.h
 *
 *  Created on: Mar 2, 2018
 *      Author: Do Yeou Ku
 */

#ifndef MAIN_H_
#define MAIN_H_

/* Standard includes. */
#include "stm32f4xx.h"
#include "stm32f4_discovery.h"

/* Task includes. */
#include "tasks/memory.h"
#include "tasks/interrupt.h"
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


#endif /* MAIN_H_ */
