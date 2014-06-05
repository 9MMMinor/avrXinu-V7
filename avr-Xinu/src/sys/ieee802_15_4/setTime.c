//
//  setTime.c
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
#include <date.h>
#include <sleep.h>
#include <avr/eeprom.h>
#include <ctype.h>

static Bool clkset = FALSE;
extern char *vers;
extern int ascdate(long time, char * str);
void frameDump(char *routine, uint8_t *p, int len);
int parseTime(long,int *,int *,int *,int *,int *,int *);

#define MACH_NAME_SIZE 10
/*
 * Where to store the clk value in EEPROM.  This is used in order
 * to remember the value across a RESET.
 */
uint32_t ee_clk __attribute__((section(".eeprom")));
uint8_t mach_name[MACH_NAME_SIZE] __attribute__((section(".eeprom")));
uint8_t macMachineName[MACH_NAME_SIZE];

int setTime(uint32_t *timevar)
{
	STATWORD ps;
	long time, time_from_eeprom;
	long lastBoot = clktime;
	static int	year, month, day, hour, minute, second;
	int m, y;
	long days;
	int len;
	char monthString[4], dateString[22];
	
	
	/*
	 * Read the value from EEPROM.  If it is not 0xffffffff (erased memory),
	 * use it as the starting value for the local time.
	 */
	if ((time_from_eeprom = eeprom_read_dword(&ee_clk)) != 0xffffffff)
		lastBoot = time_from_eeprom;
	parseTime(lastBoot,
			  &month,
			  &day,
			  &year,
			  &hour,
			  &minute,
			  &second);
	printf("Last boot date:    %2d:%02d:%02d %3s %d %4d\n", hour, minute,
		   second, Dat.dt_mnam[month], day, year);
	strncpy(monthString,Dat.dt_mnam[month],3);
	time = 0;
Parse:
	do {	//parse date string
		write(CONSOLE, (unsigned char *)"Enter date string: ", 19);
		read(CONSOLE, (unsigned char *)dateString, 22);
		len = sscanf(dateString, "%2d:%02d:%02d %3s %2d %4d", &hour, &minute, &second,
					 monthString, &day, &year);
	} while (len < 0 || len > 6);
	for (month = 0; strncmp(monthString, Dat.dt_mnam[month], 3) != 0; month++)	{
		if (month == 11)	{
			goto Parse;
		}
	}
	
	for (y = 1970; y < year; y++)	{
		days = isleap(y) ? 366 : 365;
		time += days * SECPERDY;
	}
	
	for (m = 0; m < month; m++)	{
		time += Dat.dt_msize[m] * SECPERDY;
		if (isleap(y) && (m == 1))
			time += SECPERDY;
	}
	
	time += (day-1)*SECPERDY;
	time += hour*SECPERHR;
	time += minute*SECPERMN;
	time += second;
	
	disable(ps);
	*timevar = clktime = ltim2ut(time);			// Set the system clock (clktime)
	restore(ps);
	/* Save local time */
	eeprom_write_dword(&ee_clk, time);
	printf("[EEPROM updated]\n");
	
	return OK;
}

/*
 *------------------------------------------------------------------------
 *	getname -- replaces version in src/sys/net
 *			to be replaced by radio_getname()
 *------------------------------------------------------------------------
 */

int getname(char *nam)
{
	uint8_t *p = mach_name;
	int i;
	char ch;
	
	for (i=0; i<MACH_NAME_SIZE; i++)	{
		macMachineName[i] = ch = eeprom_read_byte(p++);
		if ( isgraph(ch) )	/* any printable except space */
			continue;
		if ( ch == 0 )
			break;
		macMachineName[i] = 0;
		break;
	}
	strncpy(nam, (const char *)macMachineName, MACH_NAME_SIZE);
	return (OK);
}

/*
 *------------------------------------------------------------------------
 * getutim  --  obtain time in seconds past Jan 1, 1970, ut (gmt)
 *------------------------------------------------------------------------
 */

SYSCALL getutim(long *timvar)
{
	STATWORD ps;
	uint32_t utnow;
	
	wait(clmutex);
	if ( !clkset )
	{
		setTime(&utnow);	/* get Unix Time Now */
		clkset = TRUE;		/* right or wrong */
	}
	disable(ps);
	*timvar = clktime;
	restore(ps);
	signal(clmutex);
	return(OK);
}
