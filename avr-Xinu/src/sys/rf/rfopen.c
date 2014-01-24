/* rfopen.c - rfopen */

#include <avr-Xinu.h>
#include <network.h>

/*------------------------------------------------------------------------
 *  rfopen  --  open a remote file
 *------------------------------------------------------------------------
 */
 
int rfopen(struct devsw *devptr, char *name, char *mode)
{
	STATWORD ps;    
	struct	rfblk	*rfptr;
	int	i;
	int	mbits;
	int	devnum;

	disable(ps);
	if (strlen(name) > RNAMLEN || (mbits=ckmode(mode)) == SYSERR
	    || (i=rfalloc()) == SYSERR) {
		restore(ps);
		return(SYSERR);
	}
	rfptr = &Rf.rftab[i];
	devnum = rfptr->rf_dnum;
	strcpy(rfptr->rf_name, name);
	rfptr->rf_mode = mbits;
	rfptr->rf_pos = 0L;

	/* send remote file open request */

	if ( rfio(&devtab[devnum], FS_OPEN, NULLSTR, 0) == SYSERR ) {
		rfptr->rf_state = RFREE;
		restore(ps);
		return(SYSERR);
	}
	restore(ps);
	return(devnum);
}
