/* x_bpool.c - x_bpool */

#include <avr-Xinu.h>
#include <mark.h>
#include <bufpool.h>

/*------------------------------------------------------------------------
 *  x_bpool  -  (command bpool) format and print buffer pool information
 *------------------------------------------------------------------------
 */
 
COMMAND	x_bpool(int argc, int *argv)
//int	stdin, stdout, stderr, nargs;
//char	*args[];
{
	struct	bpool	*bpptr;
	char	str[80];
	int	i;

	for (i=0 ; i<nbpools ; i++) {
		bpptr = &bptab[i];
		sprintf(str,
		    "pool=%2d. bsize=%4d, sem=%2d, count=%d\n",
			i, bpptr->bpsize, bpptr->bpsem,
			scount(bpptr->bpsem));
		write(argv[1], (unsigned char *)str, strlen(str));
	}
	return(OK);
}
