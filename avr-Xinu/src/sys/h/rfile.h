/* rfile.h */

#ifndef __FILE_H_
#include <file.h>
#endif

/* Remote file device control block and defined constants */

/* Constants for server device control functions */

#define	RFCLEAR		1		/* Clear incoming messages	*/

/* Constants for controlling retrys for server communication */

#define	RTIMOUT		5		/* Timeout for server response	*/
#define	RMAXTRY		2		/* Number of retrys per op.	*/

/* Constants for rf pseudo-device state variable */

#define	RFREE		-1		/* This pseudo-device is unused	*/
#define	RUSED		1		/* This pseudo-device is used	*/

/* Declaration of rf pseudo-device I/O control block */

struct	rfblk	{			/* Remote file control block	*/
	int	rf_dnum;		/* File's device num in devtab	*/
	char	rf_name[RNAMLEN];	/* Name of remote file		*/
	int	rf_state;		/* State of this pseudo device	*/
	int	rf_mode;		/* FLREAD, FLWRITE or both	*/
	int	rf_mutex;		/* exclusion for this file	*/
	long	rf_pos;			/* current byte offset in file	*/
};

#ifndef	Nrf
#define	Nrf	1
#endif
struct	rfinfo	{			/* all remote server info.	*/
	int	device;			/* device descriptor for server	*/
	int	rmutex;			/* mutual exclusion for server	*/
	struct	rfblk	rftab[Nrf];	/* remote file control blocks	*/
};

extern	struct	rfinfo	Rf;

/* declarations */

SYSCALL access(char *name, int mode);
SYSCALL remove(char * name, int key);
SYSCALL rename(char * old, char * new);
int rfalloc(void);
int rfclose(struct devsw *devptr);
int rfcntl(struct devsw *devptr, int func, char *addr, char *addr2);
void rfdump(void);
int rfgetc(struct devsw *devptr);
void rfinit(struct devsw *devptr);
int rfio(struct devsw *devptr, int rop, char *buff, int len);
int rfmkpac(int rop, char *rname, long *rpos, char *buff, int len);
int rfopen(struct devsw *devptr, char *name, char *mode);
int rfputc(struct devsw *devptr, char ch);
int rfread(struct devsw *devptr, char *buff, int len);
int rfseek(struct devsw *devptr, long offset);
int rfsend(struct fphdr *fptr, int reqlen, int rplylen);
int rfwrite(struct devsw *devptr, char *buff, int len);


