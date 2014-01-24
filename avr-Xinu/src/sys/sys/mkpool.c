/* mkpool.c - mkpool */

#include <avr-Xinu.h>
#include <mark.h>
#include <bufpool.h>
#include <mem.h>


/*------------------------------------------------------------------------
 *  mkpool  --  allocate memory for a buffer pool and link together
 *------------------------------------------------------------------------
 */
SYSCALL mkpool(int bufsiz, int numbufs)

{
	STATWORD ps;    
	int	poolid;
	char *where;
	int	*getmem();
	char *a;
	int i;

#ifdef	MEMMARK
	if ( unmarked(bpmark) )
		poolinit();
#endif
	disable(ps);
	if (bufsiz<BPMINB || bufsiz>BPMAXB
	    || numbufs<1 || numbufs>BPMAXN
	    || nbpools >= NBPOOLS) {
		kprintf("mkpool: bufsiz=%d, %d %d\n", bufsiz, BPMINB, BPMAXB);
		kprintf("mkpool: numbufs=%d 1 %d\n", numbufs, BPMAXN);
		kprintf("mkpool: nbpools=%d, %d\n", nbpools, NBPOOLS);
		restore(ps);
		return(SYSERR);
	}
	if ((where= (char *) getmem((bufsiz+sizeof(int))*numbufs)) == (char *) SYSERR) {
		kprintf("mkpool: Failure to allocate %d bytes for %d buffers\n",(bufsiz+sizeof(int))*numbufs,numbufs);
		for (i=2; (a=(char *) getmem(i)) != (char *) SYSERR; i += 2) {
			freemem((struct mblock *)a,i);
		}
		kprintf("%d bytes avail to getmem\n",i-2);
		restore(ps);
		return(SYSERR);
	}
	poolid = nbpools++;
	bptab[poolid].bpnext = where;
	bptab[poolid].bpsize = bufsiz;
	bptab[poolid].bpsem = screate(numbufs);
	bufsiz+=sizeof(int);
	for (numbufs-- ; numbufs>0 ; numbufs--,where+=bufsiz)
		*( (int *) where ) = (int)(where+bufsiz);
	*( (int *) where) = (int) NULLPTR;
	restore(ps);
	return(poolid);
}
