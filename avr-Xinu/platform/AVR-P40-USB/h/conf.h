/* conf.h (GENERATED FILE; DO NOT EDIT) */

/* Device table declarations */
struct	devsw	{			/* device table entry */
	int	dvnum;
	char	*dvname;
	int	(*dvinit)(struct devsw *);
	int	(*dvopen)(struct devsw *, void *, void *);
	int	(*dvclose)(struct devsw *);
	int	(*dvread)(struct devsw *, unsigned char *, int);
	int	(*dvwrite)(struct devsw *, unsigned char *, int);
	int	(*dvseek)(struct devsw *, long);
	int	(*dvgetc)(struct devsw *);
	int	(*dvputc)(struct devsw *, unsigned char);
	int	(*dvcntl)(struct devsw *, int, void *, void *);
	void	*dvcsr;
	void	(*dviint)(void *);
	void	(*dvoint)(void *);
	void	*dvioblk;
	int	dvminor;
	};

extern	struct	devsw devtab[];		/* one entry per device */


/* Device name definitions */

#define	CONSOLE     0			/* type tty      */

/* Control block sizes */

#define	Ntty	1

#define	NDEVS	1

/* Declarations of I/O routines referenced */



/* System clock dependencies */
#define TICK		10		/* number of clock ticks per second */
/* a call of sleep10(TICK) is guaranteed to put the process to sleep for 1 second */

#define	LITTLE_ENDIAN	0x1234
#define	BIG_ENDIAN		0x4321

#define	BYTE_ORDER	LITTLE_ENDIAN

/* Configuration and Size Constants */

#define	NPROC	    10		/* number of user processes	*/
#define	NSEM	    30		/* number of semaphores		*/
//#define MEMMARK			/* define if memory marking used*/
#define	RTCLOCK				/* now have RTC support		*/
//#define STKCHK			/* resched checks stack overflow*/
//#define DEBUG				/* debug resched		*/
//#define	NETDAEMON			/* Ethernet network daemon runs	*/
#define	GATEWAY	     192,168,1,1			/* Gateway address		*/
#define	TSERVER	    "192.168.1.100:37"		/* Time server address		*/
#define	NIST_TSERVER "64.147.116.229:37"	/* Time server address	nist1-la.ustiming.org, Los Angeles, CA	*/
#define	RSERVER	    "192.168.1.100:2001"	/* Remote file server address	*/
#define	NSERVER	    "192.168.1.100:53"		/* Local Domain Name server address	*/

/* Don't affect allocation */
#define	BPMAXB	1500		/* max buffer size for mkpool (2048 default)	*/
#define BPMAXN	20			/* max # buffers in a buf pool (100 default)	*/
/* Do affect allocation */
#define NETBUFS 4		/* max # of pools (5 default)	*/
#define EMAXPAK 590		/* max packet size (576 default) */

#define	VERSION	    "AVR7.2.0 (Mar 6, 2010)"/* label printed at startup	*/

