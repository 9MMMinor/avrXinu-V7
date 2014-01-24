/* gettime.c - gettime */

#include <conf.h>
#include <kernel.h>
#include <date.h>

SYSCALL getutim(long *time);

/*
 *------------------------------------------------------------------------
 *  gettime  -  get local time in seconds past Jan 1, 1970
 *------------------------------------------------------------------------
 */
 
SYSCALL	gettime(long *timvar)
{
	long now;

	if (getutim(&now) == SYSERR)
		return(SYSERR);
	*timvar = ut2ltim(now);			/* adjust for timezone	*/
	return(OK);
}
