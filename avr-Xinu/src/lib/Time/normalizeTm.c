/**
 *  \file normalizeTm.c
 *  \project Demo
 */

//
//  Created by Michael Minor on 3/12/15.
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

#include <stdlib.h>
#include <date.h>
#include <time.h>

/*
 * https://svn.r-project.org/R/trunk/src/main/datetime.c
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 2000-2014  The R Core Team.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, a copy is available at
 *  http://www.r-project.org/Licenses/
 *
 *	Modified March 5, 2015 from the original: "static int validate_tm (stm *tm)"
 *	and renamed for use with avr-Xinu. mmm
 */

/*
 *----------------------------------------------------------------------------------------
 *	normalizeTm() - Normalize a tm structure.
 *----------------------------------------------------------------------------------------
 */

/**
 \brief Normalize the broken-down time structure pointed to by brokentime.
 		The function also fills in some components based on the values of the others.
 \param struct tm *brokentime - pointer to the broken-down time structure to be normalized.
 \return brokentime.
 */

struct tm *normalizeTm(struct tm *tm)
{
	int tmp;
	
	/* normalize seconds */
	if (tm->tm_sec < 0 || tm->tm_sec > 60) { /* 61 POSIX, 60 draft ISO C */
		tmp = tm->tm_sec/60;
		tm->tm_sec -= 60*tmp;
		tm->tm_min += tmp;
		if(tm->tm_sec < 0) {
			tm->tm_sec += 60;
			tm->tm_min--;
		}
	}
	/* normalize minutes */
	if (tm->tm_min < 0 || tm->tm_min > 59) {
		tmp = tm->tm_min/60;
		tm->tm_min -= 60*tmp;
		tm->tm_hour += tmp;
		if(tm->tm_min < 0) {
			tm->tm_min += 60;
			tm->tm_hour--;
		}
	}
	/* normalize hours -- special case 1st */
	if (tm->tm_hour == 24 && tm->tm_min == 0 && tm->tm_sec == 0) {
		tm->tm_hour = 0;
		tm->tm_mday++;
		if(tm->tm_mon >= 0 && tm->tm_mon <= 11) {
			if(tm->tm_mday > days_in_month[tm->tm_mon] +
			   ((tm->tm_mon==1 && isleap(1900+tm->tm_year) ? 1 : 0))) {
				tm->tm_mon++;
				tm->tm_mday = 1;
				if(tm->tm_mon == 12) {
					tm->tm_year++;
					tm->tm_mon = 0;
				}
			}
		}
	}
	/* normalize hours */
	if (tm->tm_hour < 0 || tm->tm_hour > 23) {
		tmp = tm->tm_hour/24;
		tm->tm_hour -= 24*tmp;
		tm->tm_mday += tmp;
		if (tm->tm_hour < 0) {
			tm->tm_hour += 24;
			tm->tm_mday--;
		}
	}
	/* normalize year */
	if (tm->tm_mon < 0 || tm->tm_mon > 11) {
		tmp = tm->tm_mon/12;
		tm->tm_mon -= 12*tmp;
		tm->tm_year += tmp;
		if (tm->tm_mon < 0) {
			tm->tm_mon += 12;
			tm->tm_year--;
		}
	}
	
	/* A limit on the loops of about 3000x round */
	//	if(tm->tm_mday < -1000000 || tm->tm_mday > 1000000) return -1;
	
	/* normalize mday */
	if (abs(tm->tm_mday) > 366) {
		/* first spin back until January */
		while (tm->tm_mon > 0) {
			--tm->tm_mon;
			tm->tm_mday += days_in_month[tm->tm_mon] +
			((tm->tm_mon==1 && isleap(1900+tm->tm_year))? 1 : 0);
		}
		/* then spin on/back by years */
		while (tm->tm_mday < 1) {
			--tm->tm_year;
			tm->tm_mday += 365 + (isleap(1900+tm->tm_year)? 1 : 0);
		}
		while (tm->tm_mday >	/* >days in the year */
			   (tmp = 365 + (isleap(1900+tm->tm_year)? 1 : 0))) {
			tm->tm_mday -= tmp;
			tm->tm_year++;
		}
	}
	while (tm->tm_mday < 1) {
		if (--tm->tm_mon < 0) {
			tm->tm_mon += 12;
			tm->tm_year--;
		}
		tm->tm_mday += days_in_month[tm->tm_mon] +
		((tm->tm_mon==1 && isleap(1900+tm->tm_year))? 1 : 0);
	}
	while (tm->tm_mday >
		   (tmp = days_in_month[tm->tm_mon] +
			((tm->tm_mon==1 && isleap(1900+tm->tm_year))? 1 : 0))) {
			   if (++tm->tm_mon > 11) {
				   tm->tm_mon -= 12;
				   tm->tm_year++;
			   }
			   tm->tm_mday -= tmp;
		   }
	_day_of_epoch(tm, 1);		/* resets tm_wday and tm_yday */
	return tm;
}
