/**
 *  \file localtime.c
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

#include <time.h>
#include <date.h>

#define TRUE 1
extern struct tm system_time;

/********************************************************************************//*!
 * \fn int function()
 * \brief Enter function description.
 * \param enter parameter
 * \return enter return
 * \return else return
 ***********************************************************************************/
/*
 *----------------------------------------------------------------------------------
 *	localtime() - Parse a time into static struct tm and return pointer to it.
 *----------------------------------------------------------------------------------
 */
/**	The localtime() function converts the time in seconds since the Epoch pointed
	to by 'time' into a broken-down time, expressed as Coordinated Universal Time
	(UTC) (formerly called Greenwich Mean Time (GMT)).
	
	The result is placed in a static variable.
	
	If the conversion is successful the function returns a pointer to the object the
	result was written into.
 
	\brief Parse a time into struct tm and return pointer to it.
	\param const time_t *time - pointer to local or system (UNIX) time
 */
struct tm * localtime(const time_t *gmt)
{
	
	parseTime(gmt, TRUE, &system_time);
	return (&system_time);
}

/*
 *----------------------------------------------------------------------------------
 *	localtime_r() - Parse a time into broken-down time at resultp; return resultp.
 *----------------------------------------------------------------------------------
 */
/**	The localtime_r() function converts the time in seconds since the Epoch pointed
	to by 'time' into a broken-down time, expressed as Coordinated Universal Time
	(UTC) (formerly called Greenwich Mean Time (GMT)).
	
	This function is similar to localtime(), except that it parses the time into a
	tm structure pointed to by the argument, 'resultp.' The localtime_r() function
	is thread-safe and returns values in a user-supplied buffer instead of
	possibly using a static data area that may be overwritten by each call.
	
	If the conversion is successful the function returns a pointer to the object the
	esult was written into, i.e., it returns resultp.
	
	\brief Parse a time into struct tm and return pointer to it.
	\param const time_t *time - pointer to local or system (UNIX) time
	\param struct tm *resultp - pointer to a tm structure to receive the result
 */
struct tm *localtime_r(const time_t *utc, struct tm *resultp)
{
	
	parseTime(utc, TRUE, resultp);
	return (resultp);
}