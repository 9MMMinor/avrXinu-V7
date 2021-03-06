/* ascdate.c - ascdate */

#include <avr-Xinu.h>
#include <date.h>

/*------------------------------------------------------------------------
 *  ascdate  -  print a given date in ascii including hours:mins:secs
 *------------------------------------------------------------------------
 */
int ascdate(long time, char * str)
{
	long	tmp;
	int	year, month, day, hour, minute, second;
	long	days;

	parseTime(time,
			  &month,
			  &day,
			  &year,
			  &hour,
			  &minute,
			  &second);
	sprintf(str, "%3s %d %4d %2d:%02d:%02d", Dat.dt_mnam[month],
		day, year, hour, minute, second);
	return(OK);
}

/*------------------------------------------------------------------------
 *  parseTime  -  parse a Xinu time into mo, day, yr, hr, min, sec
 *------------------------------------------------------------------------
 */
int
parseTime(long time,
		  int *month,
		  int *day,
		  int *year,
		  int *hour,
		  int *minute,
		  int *second)
{
	long	tmp;
	long	days;
	
	/* set year (1970-1999) */
	for ((*year)=1970 ; TRUE ; (*year)++) {
		days = isleap(*year) ? 366 : 365;
		tmp = days * SECPERDY;
		if (tmp > time)
			break;
		time -= tmp;
	}
	/* set month (0-11) */
	for ((*month)=0 ; (*month)<12 ; (*month)++) {
		tmp = Dat.dt_msize[*month] * SECPERDY;
		if (isleap(*year) && (*month == 1))
		    tmp += SECPERDY;
		if (tmp > time)
			break;
		time -= tmp;
	}
	/* set day of month (1-31) */
	*day = (int)( time/SECPERDY ) + 1;
	time %= SECPERDY;
	/* set hour (0-23) */
	*hour = (int) ( time/SECPERHR );
	time %= SECPERHR;
	/* set minute (0-59) */
	*minute = time / SECPERMN;
	time %= SECPERMN;
	/* set second (0-59) */
	*second = (int) time;
	return(OK);
}

struct	datinfo	Dat = { {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
						{"Jan", "Feb", "Mar", "Apr", "May", "Jun",
						 "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"} };
