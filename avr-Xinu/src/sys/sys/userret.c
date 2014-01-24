/* userret.c - userret */

#include <conf.h>
#include <kernel.h>

extern SYSCALL getpid();
extern SYSCALL kill();

/*------------------------------------------------------------------------
 * userret  --  entered when a process exits by return
 *------------------------------------------------------------------------
 */
SYSCALL userret(void)
{
	return ( kill( getpid() ) );
}
