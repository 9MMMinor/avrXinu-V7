/* x_devs.c - x_devs */

#include <avr-Xinu.h>
#include <avr/pgmspace.h>

const char hd1[] PROGMEM = "Num  Device   minor   CSR    i-vect.  o-vect.   cntrl blk\n";
const char hd2[] PROGMEM = "--- --------  ----- -------- -------- --------  ---------\n";

/*------------------------------------------------------------------------
 *  x_devs  -  (command devs) print main fields of device switch table
 *------------------------------------------------------------------------
 */
COMMAND	x_devs(int nargs, int *argv)
//int	stdin, stdout, stderr, nargs;
//char	*args[];
{
	struct	devsw	*devptr;
	char *str;
	int	i;

	str = (char *)malloc(60);
	fputs_P(hd1, stdout);
	fputs_P(hd2, stdout);
	for (i=0 ; i<NDEVS ; i++) {
		devptr = &devtab[i];
		sprintf_P(str, PSTR("%2d. %-9s %3d   %08x %08x %08x  %08x\n"),
			i, devptr->dvname, devptr->dvminor,
			devptr->dvcsr, 0, 0,
			devptr->dvioblk);
		write(argv[1], (unsigned char *)str, strlen(str));
	}
	free(str);
	return (OK);
}
