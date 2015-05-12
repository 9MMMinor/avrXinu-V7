/**
 *  \file day_of_epoch.c
 *  \project Demo
 */

//
//  Created by Michael Minor on 3/15/15.
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

#define THURSDAY 4

/*
 *------------------------------------------------------------------------------------------------
 *	internal function _day_of_epoch() - return the number of days since the beginning of the epoch.
 *------------------------------------------------------------------------------------------------
 */
/**	@brief Return the number of days since the beginning of the epoch.
			Reset tm_yday, tm_wday.
	@param struct tm *brokentime
	@param int normalize - FALSE (0) => do not modify brokentime;
							TRUE (1) => modify tm_yday and tm_wday.
	@return Days since the beginning of the epoch.
 */

int16_t _day_of_epoch(struct tm *tm, int norm)
{
	int16_t day, year, year0;
	int i;
	
	day = tm->tm_mday - 1;
	year0 = 1900 + tm->tm_year;
	
	for (i = 0; i < tm->tm_mon; i++) day += days_in_month[i];
	if (tm->tm_mon > 1 && isleap(year0)) day++;
	if ( norm ) tm->tm_yday = day;
	
	if (year0 > 1970) {
		for (year = 1970; year < year0; year++)
			day += days_in_year(year);
	} else if (year0 < 1970) {
		for (year = 1969; year >= year0; year--)
			day -= days_in_year(year);
	}
	if ( norm )	{
		/* weekday: Epoch day was a Thursday */
		if ((tm->tm_wday = (day + THURSDAY) % 7) < 0) tm->tm_wday += 7;
	}
	return day;
}
