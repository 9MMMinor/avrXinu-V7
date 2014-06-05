//
//  test2.c
//  802_15_4_Mib
//
//  Created by Michael Minor on 4/1/14.
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
#include <sleep.h>
#include <shell.h>

#include "mib.h"

int MLME_GET_request(int, int);
int mainShell(int, int *);
int x_pingServer(int, int *);

/*
 *------------------------------------------------------------------------
 *  main  --  run Xinu pseudo-shell with ping inside
 *------------------------------------------------------------------------
 */
int main()
{
	
	/* main has too small of a stack! */
	resume( create(mainShell, 600, 20, "shell", 0) );
//	resume( create(x_pingServer, 300, 100, "pingd", 0) );
	return 0;
}

int mainShell(int argc, int *argv)
{
//	int i;
	
//	for (i = 0; i < MAX_MAC_IDENTIFIER; i++)	{
//		MLME_GET_request(i,0);
//	}
	sleep(1);
	
	while (TRUE) {
		login(CONSOLE);
		shell(CONSOLE);
	}
	return (0);
}
