//
//  Xinu_main.c
//  HelloWorld
//
//  Created by Michael Minor on 4/5/15.
//  Copyright 2015.
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


/* avr-xinu */
#include <avr-Xinu.h>	/* includes avr-libc <stdio.h>	*/

/** @brief Xinu entrypoint.
 *
 *  This is the user entrypoint called from \a nulluser() with
 *	\b INITSTK, initial stack size; \b INITPRIO, initial priority;
 *	and \b INITNAME, process name.
 *
 *	@return The return value is ignored.
 */
int 
main( void )
{
	//
	// Enter code here
	//
	
	kprintf("kprintf: Hello World\n");
	write(TTYA,(unsigned char *)"TTYA: Hello World\n\r", 19);
	write(TTYB,(unsigned char *)"TTYB: Hello World\n\r", 19);
	printf("This is the console terminal\n");
	printf(" CONSOLE = USART%d\n", CONSOLE);
	// End enter code
	sleep(1);
	
	return 0 ;
}
