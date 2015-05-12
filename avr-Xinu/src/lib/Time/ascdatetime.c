/* ascdatetime.c - isDST, parseTime, ascdate */

#include <stdio.h>
#include <avr/pgmspace.h>
#include <time.h>
#include <date.h>

extern const int days_in_month[12];
extern const char *mon_name[12];
extern const char *day_name[7];
extern struct tm system_time;

#define SUNDAY 0
#define TRUE 1
#define FALSE 0

/*
 *------------------------------------------------------------------------
 *  isDST  -  figure out if DST applies
 *
 *	Handle daylight savings time
 *
 *------------------------------------------------------------------------
 */

#ifdef USE_DST
static int
isDST(struct tm *ptm, int Jan1)
{
	int Mar1, Nov1;							/* Day of the week 0-7 */
	int firstDay_DST, lastDay_DST;			/* Calendar day of month (1-31) */
	int i;
	
	/* Automatic DST calculation: DST is on between 2:00 AM	*/
	/*   the second Sunday of March and 2:00 AM the first	*/
	/*   sunday in November									*/
	
	/* DST is off during December, January, and Feburary and don't bother before 2006 */
	if ( (ptm->tm_mon == 11) || (ptm->tm_mon == 0) || (ptm->tm_mon == 1) ||
			(ptm->tm_year+1900 < 2006) )
		return ( FALSE );
	
	/* DST is on from April through October */
	if ( (ptm->tm_mon>2) && (ptm->tm_mon<10) )
		return ( TRUE );
	
	/* DST in March or November	*/
	/* Calculate day of the week for March 1 */
	
	Mar1 = (Jan1 + days_in_month[0] + days_in_month[1] +
				isleap(ptm->tm_year)) % 7;
	
	/* Caculate day (1-31) of the second sunday in March */
	
	if (Mar1 == SUNDAY)
		firstDay_DST = 8;			/* second sunday is 8th */
	else
		firstDay_DST = 15 - Mar1;
	
	/* Calculate day of the week for November 1		*/
	
	Nov1 = Jan1;
	for (i=0; i<10; i++) {
		Nov1 += days_in_month[i];
	}
	if (isleap(ptm->tm_year)) {
		Nov1++;
	}
	Nov1 = Nov1 % 7;
	if (Nov1 == SUNDAY)
		lastDay_DST = 1;			/* first sunday is the 1st */
	else
		lastDay_DST = 8 - Nov1;

	/* DST is off in March until 2 AM on the second Sunday */
	if ( (ptm->tm_mon == 2) &&
			( (ptm->tm_mday < firstDay_DST) || ((ptm->tm_mday == firstDay_DST) && (ptm->tm_hour < 2)) ) )
			return ( FALSE );
	
	/* DST is off in November after 2 am on first Sunday	*/
	if ( (ptm->tm_mon == 10) &&
			( (ptm->tm_mday > lastDay_DST) || ((ptm->tm_mday == lastDay_DST) && (ptm->tm_hour >= 1)) ) )
			return ( FALSE );
	
	return (TRUE);  /* rest of March or 1st part of November */
}
#endif

/*
 *------------------------------------------------------------------------
 *  parseTime  -  parse a Xinu time into a tm structure
 *------------------------------------------------------------------------
 */

/* modified
 * Michael Minor
 * Mar 1 2015
 *
 *  Object: Must be compatible with mktime.c, gmtime.c, and localtime.c
 *          Must be able to work with time_t which is signed int and
 *          interpret dates prior to the (Unix) epoch.
 */
/**		@brief Parse a Simple Time into a tm structure.
 			The Simple Time is always presumed to be seconds past the epoch in
 			UTC. A negative time is parsed to a date before the epoch. If the
 			variable 'local' is set to TRUE, the parse result is local time,
 			otherwise UTC. Time zone data are defined in 'date.h.'
 		@param const time_t *time
			Pointer to the Simple Time to parse.
 		@param const int local
			TRUE if parse is a local time.
			FALSE if parse is UTC.
 		@param struct tm *ptm
			Pointer to a structure to receive the parsed time.
 */
void
parseTime(const time_t *time, const int local, struct tm *ptm)
{
	int16_t y, tmp;
    time_t t = *time;
	
	if ( local )	{
		t = ut2ltim(t);			/* convert to local time zone */
		ptm->tm_isdst = -1;		/* daylight saving time is unknown */
		ptm->tm_gmtoff = ut2ltim(0);
		ptm->tm_zone = TIMEZONE_NAME;
	} else	{
		ptm->tm_isdst = 0;		/* Not daylight savings time */
		ptm->tm_gmtoff = 0;		/* Coordinated Universal Time */
		ptm->tm_zone = UTC_NAME;
	}
	
    int16_t day = (int16_t) (t/SECPERDY);
    int32_t left = (int32_t) (t%SECPERDY);
	
	if (left < 0)	{
		left += SECPERDY;
		day--;
	}
    
    /* set hour (0-23) */
    ptm->tm_hour = (int) (left/SECPERHR);
    left %= SECPERHR;
    /* set minute (0-59) */
    ptm->tm_min = (int) (left/SECPERMN);
    left %= SECPERMN;
    /* set second (0-59) */
    ptm->tm_sec = (int) left;
    
    /* weekday: 1970-01-01 was a Thursday */
    if ((ptm->tm_wday = ((4 + day) % 7)) < 0) ptm->tm_wday += 7;

	/* year & day within year */
    y = 1970;
    if (day >= 0)
        for ( ; day >= (tmp = days_in_year(y)); day -= tmp, y++);
    else
        for ( ; day < 0; --y, day += days_in_year(y) );
    
    y = ptm->tm_year = y - 1900;		/* set years from 1900		*/
    ptm->tm_yday = day;					/* set day of the year (1-365 or 366) */
    
    /* month within year */
    int mon;
    for (mon = 0;
         day >= (tmp = (days_in_month[mon]) + ((mon==1 && isleap(y+1900))?1:0));
            day -= tmp, mon++);
    ptm->tm_mon = mon;					/* set month (0-11)			*/
    ptm->tm_mday = day + 1;             /* set day of month (1-31)	*/

#ifdef USE_DST
	int leapyrs = (ptm->tm_year-69)/4;	/* no. leap yrs prior to this year */
	int Jan1 = (4 + (ptm->tm_year-70) + leapyrs) % 7; /* day of week Jan 1 this year */
	if ( local && isDST(ptm, Jan1) )	{
		ptm->tm_isdst = 1;						/* local daylight savings time */
		ptm->tm_gmtoff = ltim2dst(ut2ltim(0));	/* Seconds of offset */
		ptm->tm_zone = DSTZONE_NAME;
		if (++ptm->tm_hour > 23)	{
			ptm->tm_hour = 0;
			if (++ptm->tm_mday > days_in_month[ptm->tm_mon])	{
				ptm->tm_mon++;
				ptm->tm_yday++;
				ptm->tm_wday++;
				ptm->tm_wday %= 7;
				ptm->tm_mday = 1;
				/* Stop here because DST can't overflow year bound */
			}
		}
	}
#endif
}

/*
 *------------------------------------------------------------------------
 *  ascdate  -  print a given simple time in local ascii format
 *------------------------------------------------------------------------
 */
char *
ascdate(time_t t64, char * str)
{
	struct tm time;
	
	parseTime(&t64, TRUE, &time);
	sprintf_P(str, PSTR("%3s %3s %d %4d %2d:%02d:%02d %3s"), day_name[time.tm_wday], mon_name[time.tm_mon],
			time.tm_mday, time.tm_year+1900, time.tm_hour, time.tm_min, time.tm_sec, time.tm_zone);
	return(str);
}
