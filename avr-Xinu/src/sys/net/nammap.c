/* nammap.c - nammap */

#include <avr-Xinu.h>
#include <name.h>

/*------------------------------------------------------------------------
 *  nammap  -  using namespace, iteratively map name onto (newname,device) 
 *------------------------------------------------------------------------
 */

SYSCALL nammap(char *name, char *newname)
{
	STATWORD ps;    
	int	dev;
	char tmpnam[NAMLEN];

	disable(ps);
	dev = namrepl(name, newname);
	while (dev == NAMESPACE) {
		strcpy(tmpnam, newname);
		dev = namrepl(tmpnam, newname);
	}
	restore(ps);
	return(dev);
}
