/* date.h - net2xt, xt2net, isleap, ut2ltim */

/* Xinu stores time as seconds past Jan 1, 1970 (UNIX format), with	*/
/* 1 being 1 second into Jan. 1, 1970, UTC (coordinated universal time).  The	*/
/* Internet uses seconds past Jan 1, 1900 (also UTC)	*/

#define	net2xt(x)	((x)-2208988800UL)  /* convert net-to-xinu time	*/
#define	xt2net(x)	((x)+2208988800UL)  /* convert xinu-to-net time	*/

/* Days in months and month names used to format a date */

struct	datinfo	{
	int	dt_msize[12];
	char *dt_mnam[12];
	char *dt_tzone;
};

extern	struct	datinfo	Dat;

/* Constants for converting time to month/day/year/hour/minute/second	*/

#define ISLEAP(x)	(((x)%4==0 && (x)%100!=0) || (x)%400==0) /* inline */
#define days_in_year(year) (isleap(year) ? 366 : 365)
#define	SECPERDY	(60L*60L*24L)	/* one day in seconds		*/
#define	SECPERHR	(60L*60L)	/* one hour in seconds		*/
#define	SECPERMN	(60L)		/* one minute in seconds	*/

/* date doesn't understand daylight savings time (it was built in	*/
/*	Indiana where we're smart enough to realize that renumbering	*/
/*	doesn't save anything).	However, the local time zone can be		*/
/*	set to EST, CST, MST,or PST.									*/

/*
 * Daylight Savings Time in the USA
 * Energy Policy Act of 2005 established current US extended DST
 * The following TZ database entry
 * Rule    US      2007    max     -       Mar     Sun>=8  2:00    1:00    D
 * Rule    US      2007    max     -       Nov     Sun>=1  2:00    0       S
 * is applicable in the US except in Arizona and Hawaii. Indiana changed
 * their law in 2006.
 */
#define USE_DST	1			/* big overhead in parseTm */

#define	ZONE_EST	5		/* Eastern Standard time is 5	*/
#define	ZONE_CST	6		/*  hours west of England	*/
#define	ZONE_MST	7
#define	ZONE_PST	8
#define	TIMEZONE	ZONE_PST	/* timezone for this system	*/
#define TIMEZONE_NAME "PST"
#define DSTZONE_NAME "PDT"
#define UTC_NAME "UTC"

/* In-line procedures to convert universal-to-local time and vice versa	*/
/* Try not to use these; use mktime(), gmtime(), and localtime() - time.h */

#define	ut2ltim(x)	((x)-TIMEZONE*SECPERHR)
#define	ltim2ut(x)	((x)+TIMEZONE*SECPERHR)
#define ltim2dst(x) ((x)+SECPERHR)

/* declarations */
extern const int days_in_month[12];
extern const char *mon_name[12];
extern const char *day_name[7];
