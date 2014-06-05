/**
 *  \file x_set802154.c
 *  \project 802_15_4_Mib
 */

//
//  Created by Michael Minor on 6/4/14.
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
#include <stdlib.h>
#include <string.h>
#include "radio.h"
#include "mib.h"

#define ADDRESS 1
#define SOURCE 2
#define DEST 3
#define ID 4

/********************************************************************************//*!
 * \fn set [src|dst] [addr|id] 0x<value>
 * \brief Set radio Short Address or PAN-Id from the Xinu Shell.
 * \param "src" or "" sets Source parameter; "dst" sets Destination parameter.
 * \param "addr" or "" for address parameter; "id" for PAN-Id parameter.
 * \return 0
 ***********************************************************************************/

/*
 *------------------------------------------------------------------------
 *  x_set802154  -  set [src|dst] [addr|id] 0x<value>
 *------------------------------------------------------------------------
 */
COMMAND	x_set802154(int nargs, char *args[])
{
	uint8_t mode = ADDRESS;
	uint8_t location = SOURCE;
	uint16_t value;
	int n;
	
	n = 1;
	switch (--nargs)	{
		case 3:
			if (strcmp(args[n], "src") == 0)
				location = SOURCE;
			else if (strcmp(args[n], "dst") == 0)
				location = DEST;
			else
				location = 0;
			n++;
		case 2:
			if (strcmp(args[n], "addr") == 0)
				mode = ADDRESS;
			else if (strcmp(args[n], "id") == 0)
				mode = ID;
			else
				mode = 0;
			n++;
		case 1:
			value = (uint16_t)strtol(args[n], (char **)0, 16);
			break;
		default:
			mode = location = 0;
			break;
	}
	if (mode == 0 || location == 0)	{
		printf("usage: set [src|dst] [addr|id] 0x<value>\n");
		return 0;
	}
	if (location == SOURCE && mode == ID)	{
		radio_set_pan_id( (macPANId = value) );
	}
	if (location == SOURCE && mode == ADDRESS)	{
		radio_set_short_address( (macShortAddress = value) );
	}
	printf("set: %s %s 0x%04x\n", location==SOURCE?"src":"dst", mode==ADDRESS?"addr":"id", value);
	return OK;
}
