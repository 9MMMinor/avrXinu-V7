/* blkequ.c - blkequ */

#include <kernel.h>

/*
 *------------------------------------------------------------------------
 *  blkequ  -  return TRUE iff one block of memory is equal to another
 *------------------------------------------------------------------------
 */

int blkequ(register char *first, register char *second, register int16_t nbytes)

{
	while (--nbytes >= 0)
		if (*first++ != *second++)
			return(FALSE);
	return(TRUE);
}
