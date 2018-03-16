/*
 * global.h
 *
 *  Created on: Mar 16, 2018
 *      Author: Do Yeou Ku
 */

#ifndef TASKS_GLOBAL_H_
#define TASKS_GLOBAL_H_

/* Integer includes. */
#include "fatfs/integer.h"

typedef struct {
	UINT log_enabled;
	UINT file_opened;
	UINT filename_ok;
} FLAG;

FLAG file_flag;

#endif /* TASKS_GLOBAL_H_ */
