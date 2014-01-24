/* x_devs.c - x_devs */

#include <avr-Xinu.h>

char *hd1 = "Num  Device   minor   CSR    i-vect.  o-vect.   cntrl blk\n";
char *hd2 = "--- --------  ----- -------- -------- --------  ---------\n";

/*------------------------------------------------------------------------
 *  x_devs  -  (command devs) print main fields of device switch table
 *------------------------------------------------------------------------
 */
COMMAND	x_devs(int nargs, int *argv)
//int	stdin, stdout, stderr, nargs;
//char	*args[];
{
	struct	devsw	*devptr;
	char	str[60];
	int	i;

	write (argv[1], (unsigned char *)hd1, strlen(hd1) );
	write (argv[1], (unsigned char *)hd2, strlen(hd2) );
	for (i=0 ; i<NDEVS ; i++) {
		devptr = &devtab[i];
		sprintf(str, "%2d. %-9s %3d   %08x %08x %08x  %08x\n",
			i, devptr->dvname, devptr->dvminor,
			devptr->dvcsr, 0, 0,
			devptr->dvioblk);
		write(argv[1], (unsigned char *)str, strlen(str));
	}
	return (OK);
}
