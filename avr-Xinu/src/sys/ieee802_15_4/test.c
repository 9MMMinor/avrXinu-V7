//
//  test.c
//  802_15_4_Mib
//
//  Created by Michael Minor on 3/27/14.
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
#include <sleep.h>
#include <shell.h>

/*
 *------------------------------------------------------------------------
 *  main  --  start wireless ping and then run Xinu pseudo-shell
 *------------------------------------------------------------------------
 */

int main()
{
	int	wping();
	
	resume( create(wping, 380, 30, "Ping", 0) );
		
	while (TRUE) {
		login(CONSOLE);
		shell(CONSOLE);
	}
	return (0);
}
