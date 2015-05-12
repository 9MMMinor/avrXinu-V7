/**
 *  \file isleap.c
 *  \project Demo
 */

//
//  Created by Michael Minor on 3/16/15.
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

/**
 * \brief Determine whether 'year' is a leap year.
 * \param year - year (e.g. 2015) to test for leap.
 * \return TRUE (1) if 'year' is leap, else FALSE (0).
 */
int
isleap(int year)
{
//	return ((((year)%4==0 && (year)%100!=0) || (year)%400==0));
	int yy;
	
	return ( ((year & 0x03) == 0) &&
			( (((yy = year/100) * 100) != year) || ((yy & 0x03) == 0) ) );
}
