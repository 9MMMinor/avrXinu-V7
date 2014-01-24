/* getstk.c - getstk */

#include <conf.h>
#include <kernel.h>
#include <mem.h>

/*------------------------------------------------------------------------
 * getstk  --  allocate stack memory, returning address of topmost WORD
 *------------------------------------------------------------------------
 */
WORD *
getstk(unsigned int nbytes)	
{
	STATWORD ps;    
	struct	mblock	*p, *q;	/* q follows p along memlist		*/
	struct	mblock	*fits, *fitsq;
	WORD	len;

	disable(ps);
	if (nbytes == 0) {
		restore(ps);
		return( (WORD *)SYSERR );
	}
	nbytes = (unsigned int) roundmb(nbytes);
	fits = fitsq = NULLBLK;
	q = &memlist;
	for (p = q->mnext ; p != NULLBLK ; q = p,p = p->mnext)
		if ( p->mlen >= nbytes) {
			fitsq = q;
			fits = p;
		}
	if (fits == NULLBLK) {
		restore(ps);
		return( (WORD *)SYSERR );
	}
	if (nbytes == (len = fits->mlen) ) {
		fitsq->mnext = fits->mnext;
	} else {
		fits->mlen -= nbytes;
	}
	fits = (struct mblock *) ((WORD) fits + len - sizeof(WORD));
	*((WORD *) fits) = nbytes;
	restore(ps);
	return( (WORD *) fits);
}
