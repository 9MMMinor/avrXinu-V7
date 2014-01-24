/* blkcopy.c - blkcopy */

#include <stdio.h>
#include <kernel.h>

/*------------------------------------------------------------------------
 *  blkcopy  -  copy a block of memory form one location to another
 *------------------------------------------------------------------------
 */

int blkcopy(register uint8_t *to, register uint8_t *from, register int16_t nbytes)
{
	while (--nbytes >= 0)
		*to++ = *from++;
	return(OK);
}
