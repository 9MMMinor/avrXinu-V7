/*
 *  \file day_of_year.c
 *  \project Time
 */

//
//  Created by Michael Minor on 3/19/15.
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

#include <time.h>

extern const int days_in_month[12];

/**
 *	\brief Calculate the day of the year.
 *	\param int year [e.g. 2015]
 *	\param int month [0-11], month index.
 *	\param int day [1-31], calendar day of the month.
 *	\return day of the year [1-366].
 */
int
day_of_year(int year, int month, int day)
{
	int i, leap;
	
	leap = isleap(year);
	for (i = 0; i < month; i++)	{
		day += days_in_month[i];
		if ( leap && (i == 1) )
			day++;
	}
	return day;
}