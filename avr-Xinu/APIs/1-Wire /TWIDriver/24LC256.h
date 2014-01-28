//
//  24LC256.h
//  TWIDriver
//
//	Microchip 24LC256 256K I2C CMOS Serial EEPROM
//
//  Created by Michael Minor on 12/27/11.
//  Copyright (c) 2011 Michael M Minor. All rights reserved.
//

#ifndef TWIDriver__4LC256_h
#define TWIDriver__4LC256_h

#include <inttypes.h>

#define EEPROM24LC256_ADDRESS 0xA0>>1
#define EEPROM24LC256_PAGE_SIZE 64
#define EEPROM24LC256_WP 0				// Pin-7 tied to GND (Write Enabled)

int current_address_ReadEEPROM(uint8_t *data, uint8_t nbytes);
int WriteEEPROM(int address, uint8_t *data, uint8_t nbytes);
int ReadEEPROM(int address, uint8_t *data, uint8_t nbytes);
int testWriteBusyEEPROM(void);
#endif
