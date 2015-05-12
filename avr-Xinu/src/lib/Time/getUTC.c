/**
 *  \file getUTC.c
 *  \project Demo
 */

//
//  Created by Michael Minor on 3/13/15.
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

#include <date.h>
#include <time.h>

#define FALSE 0

/**
 \brief The getUTC() function converts a broken-down time structure to a simple time representation.
 
 The getUTC() function ignores the specified contents of the tm_wday, tm_yday, tm_gmtoff,
 and tm_zone members of the broken-down time structure. It uses the values of the other
 components to determine the calendar time; it is permissible for these components to have
 unnormalized values outside their normal ranges. getUTC() performs thes same function as mktime()
 but does not alter the tm structure whose pointer is passed as an argument.
 
 \param struct tm *brokentime - pointer to broken-down time structure.
 \return Seconds past Unix epoch. If the broken-down time represents a date prior to the epoch, the
 value returned is negative.
 */

time_t getUTC(struct tm *tm)
{
	time_t time;
	
	time = (tm->tm_sec + tm->tm_min*SECPERMN + tm->tm_hour*SECPERHR
			+ (time_t)_day_of_epoch(tm, FALSE) * SECPERDY);
	if (tm->tm_gmtoff == 0)
		return time;			/* UTC */
	return time-tm->tm_gmtoff;	/* convert local to UTC */
}
