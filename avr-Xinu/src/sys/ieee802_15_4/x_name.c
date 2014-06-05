/**
 *  \file x_name.c
 *  \project 802_15_4_Mib
 */

//
//  Created by Michael Minor on 6/2/14.
//  Copyright (c) 2014.
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
//

#include <avr-Xinu.h>
#include <avr/eeprom.h>
#include <ctype.h>
extern uint8_t mach_name[];
extern uint8_t macMachineName[];
#define MACH_NAME_SIZE 10

/********************************************************************************//*!
 * \fn int function()
 * \brief Enter function description.
 * \param enter parameter
 * \return enter return
 * \return else return
 ***********************************************************************************/
int
x_name(int nargs, int *argv)
{
	uint8_t *n = mach_name;
	uint8_t *p;
	int i;
	
	if (nargs == 1)	{
		printf("%s\n", macMachineName);
		return OK;
	}
	if (nargs != 2)	{
		printf("usage: name machine_name\n");
		return OK;
	}
	p = (uint8_t *)argv[1];
//	printf("echo: %s\n",p);
	uint8_t *q = macMachineName;
	for (i=0; i<MACH_NAME_SIZE-1 && isgraph(*p); i++)	{
		*q++ = *p;
		eeprom_write_byte(n++, *p++);
	}
	*q = 0;
	eeprom_write_byte(n, 0);
	return OK;
}