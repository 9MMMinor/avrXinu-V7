/* rfcntl.c - rfcntl */

#include <avr-Xinu.h>
#include <network.h>

/*
 *------------------------------------------------------------------------
 *  rfcntl  --  control the remote file server access system
 *------------------------------------------------------------------------
 */

int rfcntl(struct devsw *devptr, int func, char *addr, char *addr2)
{
	long	junk;
	int	len;

	junk = 0L;
	switch (func) {

	case RFCLEAR:
		/* clear port associated with rfserver */
#ifdef WLP_API_WIRELESS
		recvclr();	/* clear messages */
#else
		control(Rf.device, DG_CLEAR, NULL, NULL);
#endif
		return(OK);

	/* Universal file manipulation functions */

	case FLACCESS:
		return(rfmkpac(FS_ACCESS,addr,&junk,(char *)&junk,(int)addr2));

	case FLREMOVE:
		return( rfmkpac(FS_UNLINK, addr, &junk, NULLSTR, 0) );

	case FLRENAME:
		len = strlen(addr2) + 1;
		return( rfmkpac(FS_RENAME, addr, &junk, addr2, len) );
		
	case FLMKDIR:
/*		return( rfmkpac(FS_MKDIR, addr, &junk, &junk, 0775) );	*/
	
	case FLRMDIR:
/*		return( rfmkpac(FS_RMDIR, addr, &junk, &junk, 0) );		*/
		
	default:
		return(SYSERR);
	}	
}
