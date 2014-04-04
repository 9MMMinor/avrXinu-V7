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

extern struct radioinfo Radio;

/*
 *----------------------------------------------------------------------------
 *  x_net  -  (command netstat) print radio (ieee 802.15.4) status information
 *----------------------------------------------------------------------------
 */

COMMAND	x_net(int nargs, int *argv)
//int	stdin, stdout, stderr, nargs;
//char	*args[];
{
	char *str;
	
	str = malloc(80);
	sprintf(str,
			"bpool=%d, mutex/cnt=%d/%d\n",
			Radio.radiopool, Radio.nmutex, scount(Radio.nmutex));
	write(argv[1], (unsigned char *)str, strlen(str));
	sprintf(str,"Packets: recvd=%d, tossed=%d (%d for overrun)\n",
			Radio.npacket, Radio.ndrop, Radio.nover);
	write(argv[1], (unsigned char *)str, strlen(str));
	sprintf(str,"         (%d missed: no buffer space)  (%d errors)\n",
			Radio.nmiss, Radio.nerror);
	write(argv[1], (unsigned char *)str, strlen(str));
	
	free(str);
	return(OK);
}
