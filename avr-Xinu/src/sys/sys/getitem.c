/* getitem.c - getfirst, getlast */

#include <conf.h>
#include <kernel.h>
#include <q.h>

extern int dequeue();

/*------------------------------------------------------------------------
 * getfirst  --	 remove and return the first process on a list
 *------------------------------------------------------------------------
 */
int	getfirst(int head)	/* q index of head of list	*/
{
	int	proc;			/* first process on the list	*/

	if ((proc=q[head].qnext) < NPROC)
		return( dequeue(proc) );
	else
		return(EMPTY);
}



/*------------------------------------------------------------------------
 * getlast  --  remove and return the last process from a list
 *------------------------------------------------------------------------
 */
int	getlast(int tail)	/* q index of tail of list	*/
{
	int	proc;			/* last process on the list	*/

	if ((proc=q[tail].qprev) < NPROC)
		return( dequeue(proc) );
	else
		return(EMPTY);
}
