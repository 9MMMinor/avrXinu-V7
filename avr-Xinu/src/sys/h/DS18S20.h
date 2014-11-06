//
//  DS18S20.h
//  1-Wire API
//
//  Created by Michael Minor on 11/1/14.
//
//

#ifndef DS18S20_h
#define DS18S20_h

#define DS1820MODEL 0x10

#define WRITESCRATCH	0x4E
#define COPYSCRATCH		0x48
#define READSCRATCH		0xBE
#define RECALLEE		0xB8
#define CONVERTT		0x44
#define READPOWER		0xB4

double DS1820_getTemperature(uint8_t *);
void DS1820_startConversion(void);
void DS1820_readScratchPad(uint8_t *);

#endif
