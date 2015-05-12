//
//  time.h
//  Time
//
//  Created by Michael Minor on 3/3/15.
//
//

#ifndef U_TIME64_H
#define U_TIME64_H


#include <inttypes.h>

/**
 When possible, date and time functions and definitions conform to those found in
	https://www.gnu.org/software/libc/manual/html_mono/libc.html#Date-and-Time
 Most routines rely on the definition of Simple Calendar Time and Broken-down Time.
 Simple time is represented in the data type 'time_t'.
 
 'time_t' represents seconds elapsed from Midnight, Jan 1 1970 UTC (the Unix 'epoch').
 Its range allows this implementation to represent time up to 15:30:08 UTC on Sun,
 4 December 292,277,026,596. At this time the Unix time stamp will cease to work,
 as it will overflow the largest value that can be held in a signed 64-bit number.
 This could be called the Y292277026596 problem, however is not anticipated to pose a
 problem, as this is considerably longer than the time it would take the Sun to
 theoretically expand to a red giant and swallow the Earth.
 	/http://en.wikipedia.org/wiki/Sun#Earth.27s_fate
 
 The ability to make computations on dates is limited by the fact that tm_year uses
 a signed 16 bit int value starting at 1900 for the year. This limits the year to
 a maximum of 34,668 (32,768 + 1900). We call this the Y34K problem.
 */
typedef int64_t time_t;

/**
 Broken-down Time is a binary representation of calendar time separated into year, month,
 day, and so on. Broken-down time values are generally not useful for calculations, but
 they are useful for printing human readable time information. In this implementation,
 structure members may be manipulated, added to, and subtracted from so long as there is
 no member overflow, provided normalizeTm() is called prior to using the structure.
 The function, mktime(), always calls normalizeTm().
 
 A broken-down time value is always relative to a choice of time zone, and it also
 indicates which time zone that is. The tm structure contains a representation of time
 'broken down' into components of the Gregorian calendar. The range of members represent
 a normalized tm structure.
 */

struct tm
{
	int16_t tm_sec;			///< seconds after the minute - [ 0 to 59 ]
	int16_t tm_min;			///< minutes after the hour - [ 0 to 59 ]
	int16_t tm_hour;		///< hours since midnight - [ 0 to 23 ]
	int16_t tm_mday;		///< day of the month - [ 1 to 31 ]
	int16_t tm_mon;			///< months since January - [ 0 to 11 ]
	int16_t tm_year;		///< years since 1900
	int16_t tm_wday;		///< days since Sunday - [ 0 to 6 ]
	int16_t tm_yday;		///< days since January 1 - [ 0 to 365 ]
	int8_t tm_isdst;		///< DST flag -1 Unknown, 0 No, 1 Yes
	int32_t	tm_gmtoff;		///< signed (-12*3600 to +14*3600) TZ offset
	const char *tm_zone;	///< pointer to 3 char time-zone designation eg "PST"
};

/* prototypes */

int scanTimestamp(const char *ds, struct tm *);
char *makeTimestamp(char *ds, struct tm *);
struct tm *normalizeTm(struct tm *);
int16_t _day_of_epoch(struct tm *, int);
time_t getUTC(struct tm *);
void parseTime(const time_t *, const int, struct tm *);
struct tm *twilight(struct tm *time, double latitude, double longitude, int sunrise);
int day_of_year(int year, int month, int day);
char * ascdate(time_t, char *);
time_t mktime(struct tm *);
int isleap(int);
struct tm *gmtime(const time_t *gmt);
struct tm *gmtime_r(const time_t *utc, struct tm *resultp);
struct tm *localtime(const time_t *gmt);
struct tm *localtime_r(const time_t *utc, struct tm *resultp);
time_t time(time_t *result);
double calcSunrise(int day, int month, int year, double latitude, double longitude);
double calcSunset(int day, int month, int year, double latitude, double longitude);

#endif
