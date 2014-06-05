/* x_kill.c - x_kill */

#include <avr-Xinu.h>
#include <shell.h>

/*------------------------------------------------------------------------
 *  x_kill  -  (command kill) terminate a process
 *------------------------------------------------------------------------
 */
BUILTIN	x_kill(int nargs, char *args[])
{
	int	pid;

	if (nargs != 2) {
		printf("use: kill process-id\n");
		return(SYSERR);
	}
	if ( (pid=atoi(args[1])) == getpid() )
		printf("Shell killed\n");
	return( kill(pid) );
}
