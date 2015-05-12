/**
 *  \file time.c
 *  Date and Time
 */

//
//  Created by Michael Minor on 12/23/14.
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
#include <time.h>
#include <date.h>
#include <avr/eeprom.h>

#define MACH_NAME_SIZE 10
/*
 * Where to store the clk value in EEPROM.  This is used in order
 * to remember the value across a RESET.
 */
uint8_t ee_clk[8] __attribute__((section(".eeprom")));
char ee_timestamp[30] __attribute__((section(".eeprom")));
uint8_t mach_name[MACH_NAME_SIZE] __attribute__((section(".eeprom")));
uint8_t macMachineName[MACH_NAME_SIZE];


static Bool clkset = FALSE;

/** \brief Get current calendar time.
 *
 *	The time function returns the current calendar time as a value of type \b time_t.
 *	If the argument \b result is not a null pointer, the calendar time value is also
 *	stored in \b *result. If the current calendar time is not available, the value
 *	(time_t)(-1) is returned.
 *
 * \param time_t *result
 * \return the current calendar time as a value of type time_t.
 * \return else return (time_t)SYSERR.
 */
time_t time(time_t *result)
{
	time_t now;
	
	if (getutim(&now) == SYSERR)
		return((time_t)SYSERR);
	if (result != (time_t *)0)
		*result = now;
	return (now);
}

/*
 *------------------------------------------------------------------------
 * getutim  --  obtain time in seconds past Jan 1, 1970, UTC (gmt)
 *------------------------------------------------------------------------
 */
SYSCALL getutim(time_t *timvar)
{
	STATWORD ps;
	int ret;
	time_t utnow;		/* UTC as seconds past Thursday Jan 1, 1970 */

	wait(clmutex);
	ret = OK;
	if ( !clkset )
		{
#ifdef ETHERNET
		/*
		 * Read the Network UTC value from the Internet Time Server.  Convert to host long,
		 * then to Xinu (UNIX) time, and store in 'clktime.'
		 */
		int dev;
		if ((dev=(int)open(INTERNET, NIST_TSERVER, ANYLPORT)) == SYSERR ||
		    control(dev,DG_SETMODE,(void *)(DG_TMODE|DG_DMODE),(void *)0) == SYSERR)
			{
			panic("can't open time server");
			ret = SYSERR;
			}
		write(dev, (unsigned char *)"Xinu", 4);	/* send junk packet to prompt */
		if (read(dev,(unsigned char *)&utnow,sizeof(utnow)) != TIMEOUT)
			{
			disable(ps);
			clktime = net2xt( net2hl(utnow) );
			restore(ps);
			}
		else
			{
			kprintf("No response from time server\n");
			ret = SYSERR;
			}
		close(dev);
#else
		/*
		 * Read the UTC value from EEPROM.  If it is not 0xffffffff (erased memory),
		 * use it as the starting value for the local time.
		 */
		struct tm *ctime = (struct tm *)malloc(sizeof(struct tm));
		char *ds = (char *)malloc(30);
		time_t lastBoot = 0;
		char dateString[22];
		int len, month, year;
		
		eeprom_read_block(ds, ee_timestamp, 30); /* read the last timestamp string */
			if ( scanTimestamp(ds, ctime) == 1 )	{
				lastBoot = mktime(ctime);
			}
		localtime_r(&lastBoot, ctime);
		printf("Last boot date:    %2d:%02d:%02d %02d-%02d-%4d\n", ctime->tm_hour, ctime->tm_min,
		   ctime->tm_sec, ctime->tm_mon+1, ctime->tm_mday, ctime->tm_year+1900);

		do {	//parse date string
			write(CONSOLE, (unsigned char *)"Enter local date:  ", 19);
			read(CONSOLE, (unsigned char *)dateString, 22);
			len = sscanf(dateString, "%2d:%02d:%02d %02d-%02d-%4d", &ctime->tm_hour, &ctime->tm_min, &ctime->tm_sec,
					 &month, &ctime->tm_mday, &year);
		} while ( len != 6 );

		ctime->tm_mon = month-1;
		ctime->tm_year = year-1900;
		utnow = mktime(ctime);
			/* Save UTC time */
		disable(ps);
		clktime = utnow;			// Set the system clock (clktime) UTC
		restore(ps);
		eeprom_update_block(makeTimestamp(ds,ctime), ee_timestamp, 30);
		printf("[EEPROM updated]\n");
		free(ctime);
		free(ds);
		}
#endif
	clkset = TRUE;		/* right or wrong */
	disable(ps);
	*timvar = clktime;
	restore(ps);
	signal(clmutex);
	return(ret);
}
