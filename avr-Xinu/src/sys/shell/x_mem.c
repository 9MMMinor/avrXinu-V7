/* x_mem.c - x_mem */

#include <avr-Xinu.h>
#include <mem.h>
#include <avr/io.h>

extern char *_etext;
extern char *__bss_start, *__bss_end;
extern char *__data_start, *__data_end;
extern char *_end;



/*
 *------------------------------------------------------------------------
 *  x_mem  -  (command mem) print memory use and free list information
 *------------------------------------------------------------------------
 */

COMMAND	x_mem(int narg, int *varg)
//int	stdin, stdout, stderr, nargs;
//char	*args[];
{
	int	i;
	struct	mblock	*mptr;
	char str[80];
	unsigned free;
	unsigned avail;
	unsigned stkmem;

	/* calculate current size of free memory and stack memory */

	for( free=0,mptr=memlist.mnext ; mptr!=(struct mblock *)NULL ;
	     mptr=mptr->mnext)
		free += mptr->mlen;
	for (stkmem=0,i=0 ; i<NPROC ; i++) {
		if (proctab[i].pstate != PRFREE)
			stkmem += (unsigned)proctab[i].pstklen;
	}
	sprintf(str,
			"Flash memory: %ul bytes total, %u text\n",
			1L + (unsigned long)FLASHEND, (unsigned) &_etext);
	write(varg[1], str, strlen(str));
	sprintf(str,
			"RAM memory: %u bytes RAM, %u registers %u data, %u bss\n",
			1 + (unsigned)RAMEND, (unsigned) &__data_start,
			(unsigned) &__data_end - (unsigned) &__data_start,
			(unsigned) &__bss_end - (unsigned) &__bss_start);
	write(varg[1], str, strlen(str));
	avail = (unsigned)RAMEND - (unsigned) &_end + 1;
	sprintf(str," initially: %5u avail\n", avail);
	write(varg[1], str, strlen(str));
	sprintf(str," presently: %5u avail, %5u stack, %5u heap\n",
		free, stkmem, avail - stkmem - free);
	write(varg[1], str, strlen(str));
	sprintf(str," free list:\n");
	write(varg[1], str, strlen(str));
	for( mptr=memlist.mnext ; mptr!=(struct mblock *)NULL ;
	     mptr=mptr->mnext) {
		sprintf(str,"   block at 0x%4x, length %5u (0x%x)\n",
			mptr, mptr->mlen, mptr->mlen);
		write(varg[1], str, strlen(str));
	}
	return(OK);
}
