/* x_ps.c - x_ps */

#include <avr-Xinu.h>

static char hd1[] = "pid   name   state prio  stack range  stack length   sem message\n";
static char hd2[] = "--- -------- ----- ---- ------------- ------------   --- -------\n";
static char *pstnams[] = {"curr ","free ","ready","recv ",
			    "sleep","susp ","wait ","rtim "};
//int	psavsp;

/*------------------------------------------------------------------------
 *  x_ps  -  (command ps) format and print process table information
 *------------------------------------------------------------------------
 */
 
COMMAND	x_ps(int narg, int *argv)
//int	stdin, stdout, stderr, nargs;
//char	*args[];
{
	int	i;
	char	str[80];
	struct	pentry	*pptr;
	unsigned char *currstk;

	write(argv[1], (unsigned char *)hd1, strlen(hd1));
	write(argv[1], (unsigned char *)hd2, strlen(hd2));
	for (i=0 ; i<NPROC ; i++) {
		if ((pptr = &proctab[i])->pstate == PRFREE)
			continue;
		sprintf(str, "%3d %8s %s ", i, pptr->pname,
			pstnams[pptr->pstate-1]);
		write(argv[1], (unsigned char *)str, strlen(str));
		sprintf(str, "%4d %06x-%06x ", pptr->pprio,  pptr->plimit,
			(unsigned)pptr->pbase + 1);
		write(argv[1], (unsigned char *)str, strlen(str));
		currstk = (unsigned char *)((uint16_t)pptr->pregs[SSP_H]<<8 | ((uint16_t)pptr->pregs[SSP_L]&0xFF));
		if (currstk < pptr->plimit || currstk > pptr->pbase)
			sprintf(str, " OVERFLOWED     ");
		else
			sprintf(str, "%5d /%5d    ", pptr->pbase - currstk,
				pptr->pbase - pptr->plimit + sizeof(int));
		write(argv[1], (unsigned char *)str, strlen(str));
		if (pptr->pstate == PRWAIT)
			sprintf(str, "%2d", pptr->psem);
		else
			sprintf(str, "- ");
		write(argv[1], (unsigned char *)str, strlen(str));
		if (pptr->phasmsg)
			sprintf(str, "%06x\n", pptr->pmsg);
		else
			sprintf(str, "   -\n");
		write(argv[1], (unsigned char *)str, strlen(str));
	}
	return (OK);
}




