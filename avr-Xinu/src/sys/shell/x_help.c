/* x_help.c - x_help */

#include <avr-Xinu.h>
#include <shell.h>
#include <cmd.h>

#define	COLUMNS	4			/* number of columns to print	*/

/*------------------------------------------------------------------------
 *  x_help  -  (command help) print possible command names for user
 *------------------------------------------------------------------------
 */
COMMAND	x_help(int nargs, int *varg)
//int	stdin, stdout, stderr, nargs;
//char	*args[];
{
	int	inc;			/* command names per column	*/
	int	i;			/* move through printed rows	*/
	int	j;			/* move across printed columns	*/
	FILE * stream = stdout;


	if ( (inc=(Shl.shncmds+COLUMNS-1)/COLUMNS) <= 0)
		inc = 1;
	fprintf(stream, "Commands are:\n");
	for(i=0 ; i<inc && i<Shl.shncmds ; i++) {
		fprintf(stream, "  ");
		for (j=i ; j<Shl.shncmds ; j+=inc)
			fprintf(stream, "%-16s", cmds[j].cmdnam);
		fprintf(stream, "\n");
	}
	return(OK);
}
