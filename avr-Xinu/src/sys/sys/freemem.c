/* freemem.c - freemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>


/*------------------------------------------------------------------------
 *  freemem  --  free a memory block, returning it to memlist
 *------------------------------------------------------------------------
 */
SYSCALL	freemem(struct mblock *block, unsigned size)
{
	STATWORD ps;    
	struct mblock *p, *q;
	unsigned top;

	if (size==0 || (unsigned)block>(unsigned)__malloc_heap_end
	    || ((unsigned)block)<((unsigned) __malloc_heap_start))
		return(SYSERR);
	size = (unsigned)roundmb(size);
	disable(ps);
	for( p = memlist.mnext, q = &memlist; p != NULLBLK && p < block ; q = p, p = p->mnext )
		;
	if (((top=q->mlen+(unsigned)q)>(unsigned)block && q!= &memlist) ||
	    (p!=NULLBLK && (size+(unsigned)block) > (unsigned)p) ) {
		restore(ps);
		return(SYSERR);
	}
	if ( q!= &memlist && top == (unsigned)block )
			q->mlen += size;
	else {
		block->mlen = size;
		block->mnext = p;
		q->mnext = block;
		q = block;
	}
	if ( (unsigned)( q->mlen + (unsigned)q ) == (unsigned)p) {
		q->mlen += p->mlen;
		q->mnext = p->mnext;
	}
	restore(ps);
	return(OK);
}
