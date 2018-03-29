/*
 * bmp280.h
 *
 *  Created on: Mar 5, 2018
 *      Author: Do Yeou Ku
 */

#ifndef TASKS_BMP280_H_
#define TASKS_BMP280_H_


/**
	SPI 4 wire interface
	Maximum frequency (VDDIO>1.75): 	10 MHz
	Automatic selection (CPOL,CPHA): 	default (0,0) & (1,1)
	CSB status: 						idle 1 & active 0
**/

//------------------------------DEVICE_ID------------------------------//

// eight-bit device ID are PRS = 0x58
#define BMX055_PRS_ID  	0x58		// Address of BMP280

//------------------------------REGISTERS------------------------------//
// Barometer registers
#define BMP280_PRS_CALIB00	0x88   // should return 0x58
#define BMP280_PRS_CALIB01	0x89
#define BMP280_PRS_CALIB02	0x8A
#define BMP280_PRS_CALIB03	0x8B
#define BMP280_PRS_CALIB04	0x8C
#define BMP280_PRS_CALIB05	0x8D
#define BMP280_PRS_CALIB06	0x8E
#define BMP280_PRS_CALIB07	0x8F
#define BMP280_PRS_CALIB08	0x90
#define BMP280_PRS_CALIB09	0x91
#define BMP280_PRS_CALIB10	0x92
#define BMP280_PRS_CALIB11	0x93
#define BMP280_PRS_CALIB12	0x94
#define BMP280_PRS_CALIB13	0x95
#define BMP280_PRS_CALIB14	0x96
#define BMP280_PRS_CALIB15	0x97
#define BMP280_PRS_CALIB16	0x98
#define BMP280_PRS_CALIB17	0x99
#define BMP280_PRS_CALIB18	0x9A
#define BMP280_PRS_CALIB19	0x9B
#define BMP280_PRS_CALIB20	0x9C
#define BMP280_PRS_CALIB21	0x9D
#define BMP280_PRS_CALIB22	0x9E
#define BMP280_PRS_CALIB23	0x9F
#define BMP280_PRS_CALIB24	0xA0
#define BMP280_PRS_CALIB25	0xA1

#define BMP280_PRS_CHIP_ID		0xD0
#define BMP280_PRS_RESET		0xE0
#define BMP280_PRS_STATUS		0xF3
#define BMP280_PRS_CTRL_MEAS	0xF4
#define BMP280_PRS_CONFIG		0xF5
#define BMP280_PRS_PRESS_MSB	0xF7
#define BMP280_PRS_PRESS_LSB	0xF8
#define BMP280_PRS_PRESS_XLSB	0xF9
#define BMP280_PRS_TEMP_MSB		0xFA
#define BMP280_PRS_TEMP_LSB		0xFB
#define BMP280_PRS_TEMP_XLSB	0xFC

//---------------------BAROMETER REGISTER PIN_OUTS---------------------//
// BMP280_PRS_CHIP_ID			0xD0
#define PRS_CHIP_ID				0x58

// BMP280_PRS_RESET				0xE0
#define PRS_RESET_RESET			0xB6

// BMP280_PRS_STATUS			0xF3
#define PRS_STATUS_MEASURING	0x04
#define PRS_STATUS_IM_UPDATE	0x01

// BMP280_PRS_CTRL_MEAS			0xF4
#define PRS_CTRL_OSRS_T			0xE0
#define PRS_CTRL_OSRS_P			0x1C
#define PRS_CTRL_MODE			0x03

#define PRS_CTRL_OSRS_T_SKIP	0x00
#define PRS_CTRL_OSRS_T_1		0x20
#define PRS_CTRL_OSRS_T_2		0x40
#define PRS_CTRL_OSRS_T_4		0x60
#define PRS_CTRL_OSRS_T_8		0x80
#define PRS_CTRL_OSRS_T_16		0xA0

#define PRS_CTRL_OSRS_P_SKIP	0x00
#define PRS_CTRL_OSRS_P_1		0x04
#define PRS_CTRL_OSRS_P_2		0x08
#define PRS_CTRL_OSRS_P_4		0x0C
#define PRS_CTRL_OSRS_P_8		0x10
#define PRS_CTRL_OSRS_P_16		0x14

// BMP280_PRS_CONFIG			0xF5
#define PRS_CONFIG_T_SB			0xE0
#define PRS_CONFIG_FILTER		0x1C
#define PRS_CONFIG_SPI3W_EN		0x01

#define PRS_CONFIG_T_SB_500us	0x00
#define PRS_CONFIG_T_SB_62_5ms	0x20
#define PRS_CONFIG_T_SB_125ms	0x40
#define PRS_CONFIG_T_SB_250ms	0x60
#define PRS_CONFIG_T_SB_500ms	0x80
#define PRS_CONFIG_T_SB_1s		0xA0
#define PRS_CONFIG_T_SB_2s		0xC0
#define PRS_CONFIG_T_SB_4s		0xE0

#define PRS_CONFIG_FILTER_OFF	0x00
#define PRS_CONFIG_FILTER_2		0x04
#define PRS_CONFIG_FILTER_4		0x08
#define PRS_CONFIG_FILTER_8		0x10
#define PRS_CONFIG_FILTER_16	0x1C

// BMP280_PRS_PRESS_MSB		0xF7
#define PRS_PRESS_MSB_MSB	0xFF

// BMP280_PRS_PRESS_LSB		0xF8
#define PRS_PRESS_LSB_LSB	0xFF

// BMP280_PRS_PRESS_XLSB	0xF9
#define PRS_PRESS_XLSB_XLSB	0xF0

// BMP280_PRS_TEMP_MSB		0xFA
#define PRS_TEMP_MSB_MSB	0xFF

// BMP280_PRS_TEMP_LSB		0xFB
#define PRS_TEMP_LSB_LSB	0xFF

// BMP280_PRS_TEMP_XLSB		0xFC
#define PRS_TEMP_XLSB_XLSB	0xF0


#endif /* TASKS_BMP280_H_ */
