/* wakeup.c - wakeup */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>

extern int ready();
extern int getfirst();
extern int resched();

/*------------------------------------------------------------------------
 * wakeup  --  called by clock interrupt dispatcher to awaken processes
 *------------------------------------------------------------------------
 */
INTPROC	wakeup(void)
{
	while (nonempty(clockq) && firstkey(clockq) <= 0)
		ready(getfirst(clockq),RESCHNO);
	if ( (slnempty = nonempty(clockq)) )
		sltop = (int *volatile) &q[q[clockq].qnext].qkey;
	resched();
}
