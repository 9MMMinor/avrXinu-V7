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

static Bool clkset = FALSE;
extern char *vers;
extern int ascdate(long time, char * str);

int setTime(uint32_t *timevar)
{
	STATWORD ps;
	long time;
	long lastBoot = clktime;
	int day, mo, yr, hour, min, sec;
	int month, year;
	long days;
	int len;
	char monthString[4], dateString[22];
	
	time = 0;
//	ReadEEPROM(128, (uint8_t *)&lastBoot, 4);	// get boot local time.
	ascdate(lastBoot, dateString);
	printf("Last boot date:    %s\n", dateString);
	
Parse:
	do {	//parse date string
		write(CONSOLE, (unsigned char *)"Enter date string: ", 19);
		read(CONSOLE, (unsigned char *)dateString, 22);
		len = sscanf(dateString, "%3s %2d %4d %2d:%02d:%02d", monthString, &day, &yr,
					 &hour, &min, &sec);
	} while (len != 6);
	for (mo = 0; strncmp(monthString, Dat.dt_mnam[mo], 3) != 0; mo++)	{
		if (mo == 11) goto Parse;
	}
	
	for (year = 1970; year < yr; year++)	{
		days = isleap(year) ? 366 : 365;
		time += days * SECPERDY;
	}
	
	for (month = 0; month < mo; month++)	{
		time += Dat.dt_msize[month] * SECPERDY;
		if (isleap(year) && (month == 1))
			time += SECPERDY;
	}
	
	time += (day-1)*SECPERDY;
	time += hour*SECPERHR;
	time += min*SECPERMN;
	time += sec;
	
	disable(ps);
	*timevar = clktime = ltim2ut(time);			// Set the system clock (clktime)
	restore(ps);
//	WriteEEPROM(128, (uint8_t *)&time, 4);		// Save local time
	
	return OK;
}


int getname(char *nam)
{
	*nam = '\0';
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
