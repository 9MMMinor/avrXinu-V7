//
//  x_net.c
//  802_15_4_Mib
//
//  Created by Michael Minor on 3/29/14.
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

/* x_net.c - x_net */

#include <avr-Xinu.h>
#include "radioIO.h"

extern struct radioinfo Net;

/*
 *----------------------------------------------------------------------------
 *  x_net  -  (command netstat) print radio (ieee 802.15.4) status information
 *----------------------------------------------------------------------------
 */

COMMAND	x_net(int nargs, int *argv)
{
	int i;

	if ( nargs > 1 )		{
		printf("%s ignore arguments: ", argv[0]);
		for (i = 1; i < nargs; i++)		{
			printf("%s%s", argv[i], (i < nargs-1) ? " " : "");
		}
		printf("\n");
	}
	printf("bpool=%d, mutex/cnt=%d/%d\n",
			Net.radiopool, Net.nmutex, scount(Net.nmutex));
	printf("Packets: Total=%d, data=%d beacon=%d mac command=%d\n",
			Net.npacket, Net.ndata, Net.nbeacon, Net.ncmd);
	printf("   Data missed: (%d no buffer space)  (%d address not matched)\n",
			Net.nover, Net.ndrop);
	printf("   Errors: %d\n", Net.nerror);
	
	return(OK);
}
