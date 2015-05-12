/**
 *  \file scanTime.c
 *  \project Demo
 */

//
//  Created by Michael Minor on 3/9/15.
//  Copyright (c) 2015.
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

#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <date.h>
#include <time.h>

extern const char *day_name[];
extern const char *mon_name[];

const PROGMEM char scan_str[] = "%3s %3s %02d %02d:%02d:%02d UTC %4d";

/**	\brief Scan an ascii timestamp compatible with the Host command 'date -u.'
	
	The function scans the string at 'ds' into the broken-down time structure
	at 'tm.'  The string must be in the following format:
 
			Tue Mar 10 11:00:29 UTC 2015
	
	The timestamp created by makeTimestamp() can be scanned by scanTimestamp().
	
	 \param ds is a pointer to an ascii timestamp.
	 \param tm is a pointer to a broken-down time structure.
	 \return -1 if the scan fails; 1 if OK.
 */
int
scanTimestamp(const char *ds, struct tm *tm)
{
	int len;
	int16_t year;
	char Day[4], Month[4];
	
	len = sscanf_P(ds, scan_str, Day, Month, &tm->tm_mday,
				&tm->tm_hour, &tm->tm_min, &tm->tm_sec, &year);
	if ( len != 7 )
		return (-1);	/* error (SYSERR) */
	for (tm->tm_wday = 0; tm->tm_wday < 7; tm->tm_wday++)
		if (strncmp(Day, day_name[tm->tm_wday], 3) == 0)
			break;
	for (tm->tm_mon = 0; tm->tm_mon < 11; tm->tm_mon++)
		if (strncmp(Month, mon_name[tm->tm_mon], 3) == 0)
			break;
	if (tm->tm_wday == 8 || tm->tm_mon == 12)
		return (-1);
	tm->tm_year = year - 1900;
	tm->tm_gmtoff = 0;
	tm->tm_isdst = 1;
	return (1);			/* OK */
}

/**	\brief Make an ascii timestamp compatible with the Host command 'date -u'.
	
	The function prints the broken-down time structure at 'tm' into the string
	at 'ds.' The result will be in the following format:
 
		Tue Mar 10 11:00:29 UTC 2015
	
	The timestamp created by makeTimestamp() can be scanned by scanTimestamp().
	
	\param ds is a pointer to an ascii string to receive the timestamp.
	\param tm is a pointer to a broken-down time structure which is converted to the ascii timestamp.
	\return a pointer to the timestamp string.
 */
char *
makeTimestamp(char *ds, struct tm *tm)
{
	
	if (tm->tm_gmtoff != 0)	{
		tm->tm_sec -= tm->tm_gmtoff;
		tm->tm_gmtoff = 0;
		normalizeTm(tm);
	}
	sprintf_P(ds, scan_str, day_name[tm->tm_wday],
			mon_name[tm->tm_mon], tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, tm->tm_year+1900);
	return (ds);
}

