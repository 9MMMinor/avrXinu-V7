/*------------------------------------------------------------------------
 *	panic  --  panic and abort XINU
 *------------------------------------------------------------------------
 */

#include <kernel.h>

void panic (char *msg)
{
	STATWORD ps;    
	disable(ps);
	kprintf("Panic: %s\n", msg);
	while (1)
		{}
	restore(ps);
}

