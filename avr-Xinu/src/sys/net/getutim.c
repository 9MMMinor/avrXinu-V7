/* getutim.c - getutim */

#include <avr-Xinu.h>
#include <sleep.h>
#include <date.h>
#include <network.h>

static Bool clkset = FALSE;

/*
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * There is a race condition with netin/netout when getutim is called
 * during initialization. Try calling getutim from a lower priority
 * process: set_Utime. *mmm*
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

/*
 *------------------------------------------------------------------------
 * getutim  --  obtain time in seconds past Jan 1, 1970, ut (gmt)
 *------------------------------------------------------------------------
 */

SYSCALL getutim(long *timvar)
{
	STATWORD ps;
	int	dev, ret;
	uint32_t utnow;

	wait(clmutex);
	ret = OK;
	if ( !clkset )
		{
		if ((dev=(int)open(INTERNET, NIST_TSERVER, ANYLPORT)) == SYSERR ||
		    control(dev,DG_SETMODE,(void *)(DG_TMODE|DG_DMODE),(void *)0) == SYSERR)
			{
			panic("can't open time server");
			ret = SYSERR;
			}
		write(dev, (unsigned char *)"Xinu", 4);	/* send junk packet to prompt */
		if (read(dev,(unsigned char *)&utnow,sizeof(utnow)) != TIMEOUT)
			{
			disable(ps);
			clktime = net2xt( net2hl(utnow) );
			restore(ps);
			}
		else
			{
			kprintf("No response from time server\n");
			ret = SYSERR;
			}
		close(dev);
		clkset = TRUE;		/* right or wrong */
		}
	disable(ps);
	*timvar = clktime;
	restore(ps);
	signal(clmutex);
	return(ret);
}


PROCESS set_Utime(int arg, int * argv)
{
	int32_t utnow;
	
	if ( getutim(&utnow) == OK )
		kprintf("system clock set\n");
	return 0;
}
