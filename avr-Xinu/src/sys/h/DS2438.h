//
//  DS2438.h
//  TWIDriver
//
//  Created by Michael Minor on 2/14/13.
//  Copyright (c) 2013 Michael M Minor.
//
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef DS2438_h
#define DS2438_h

#define DS2438MODEL 0x26

#define WRITESCRATCH   0x4E
#define COPYSCRATCH    0x48
#define READSCRATCH    0xBE
#define RECALLSCRATCH  0xB8
#define CONVERTT       0x44
#define CONVERTV       0xB4

// Scratchpad locations - Page 0
#define STATUS    0
#define TEMP_LSB  1
#define TEMP_MSB  2
#define VOLT_LSB  3
#define VOLT_MSB  4
#define CURR_LSB  5
#define CURR_MSB  6
#define THRESH    7

// Page 1
#define ETM_BYTE_0	0
#define ETM_BYTE_1	1
#define ETM_BYTE_2	2
#define ETM_BYTE_3	3
#define ICA			4
#define OFFSET_LSB	5
#define OFFSET_MSB	6

// Page 2
#define DISCONNECT_BYTE_0	0
#define DISCONNECT_BYTE_1	1
#define DISCONNECT_BYTE_2	2
#define DISCONNECT_BYTE_3	3
#define ENDOFCHARGE_BYTE_0	4
#define ENDOFCHARGE_BYTE_1	5
#define ENDOFCHARGE_BYTE_2	6
#define ENDOFCHARGE_BYTE_3	7

// Page 6
#define VAD_CAL_BYTE_0	0
#define VAD_CAL_BYTE_1	1
#define VAD_CAL_BYTE_2	2
#define VAD_CAL_BYTE_3	3

// Page 7
#define CCA_LSB			4
#define CCA_MSB			5
#define DCA_LSB			6
#define DCA_MSB			7

// Status/Configuration Register bit locations
#define STATUS_IAD	0
#define STATUS_CA	1
#define STATUS_EE	2
#define STATUS_AD	3
#define STATUS_TB	4
#define STATUS_NVB	5
#define STATUS_ADB	6

//globals
extern int DS2438Error;

//prototypes
int SetupAtoD(uint8_t config, unsigned char *LaserROM);
double Get_Temperature(unsigned char *LaserROM);
double ReadAtoD(unsigned char *LaserROM);
double Get_Current(unsigned char *LaserROM);
int getDS2438Voltage(unsigned char *LaserROM);
int Read_DS2438Page(unsigned char *LaserROM, int page, uint8_t *pageByte);
int Write_DS2438Page(unsigned char *LaserROM, int page, uint8_t *pageByte);

#endif
