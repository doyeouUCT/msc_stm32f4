/*
 * sensor.h
 *
 *  Created on: Mar 5, 2018
 *      Author: Do Yeou Ku
 */

#ifndef TASKS_SENSORIO_H_
#define TASKS_SENSORIO_H_

#include "stm32f4xx.h"
#include "bosch_integer.h"
#include "bmx055.h"
#include "bmp280.h"

#define RTK_IMU_SENSOR

#ifdef RTK_IMU_SENSOR
/* ID check enable */
#define BMX055_ACC_ID_CHECK
#define BMX055_GYRO_ID_CHECK
//#define BMX055_MAG_ID_CHECL
/* Configure enable */
#define BMX055_ACC_DATA_READY
#define BMX055_GYRO_DATA_READY
//#define BMX055_MAG_DATA_READY
#endif

/* Status of Sensor Functions */
typedef BYTE	SSTATUS;

typedef enum {
	ACCEL	= 0,
	GYRO	= 1,
	MAG		= 2,
} SENSOR;

/* Results of Sensor Functions */
typedef enum {
	SENS_OK = 0,		/* 0: Successful */
	SENS_ERROR,			/* 1: R/W Error */
	SENS_NOTRDY,		/* 2: Not Ready */
	SENS_PARERR,		/* 3: Invalid Parameter */
	SENS_IDERR,			/* 4: Invalid ID */
	SENS_CONFERR,		/* 5: Configuration Error */
	SENS_INTCONFERR,	/* 6: Interrupt Configuration Error */
} SRESULT;

typedef struct{
	BYTE accel_rate;
	BYTE accel_range;
	BYTE gyro_rate;
	BYTE gyro_range;
	BYTE mag_rate;
	BYTE mag_range_xy;
	BYTE mag_range_z;
} SCONFIG;

#define ACCEL_DATA_LENGTH	7
#define GYRO_DATA_LENGTH	6
#define MAG_DATA_LENGTH		8

SSTATUS sensor_initialize (volatile SCONFIG*);
SSTATUS sensor_deinitialize (void);
SSTATUS accel_read (BYTE*, UINT);
SSTATUS gyro_read (BYTE*, UINT);
SSTATUS mag_read (BYTE*, UINT);

#endif /* TASKS_SENSORIO_H_ */
