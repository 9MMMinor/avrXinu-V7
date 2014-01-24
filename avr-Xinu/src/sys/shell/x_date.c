/* x_date.c - x_date */

#include <avr-Xinu.h>
#include <sleep.h>
#include <date.h>

/*
 *------------------------------------------------------------------------
 *  x_date  -  (command date) print the date and time
 *------------------------------------------------------------------------
 */
 
COMMAND	x_date(int nargs, int *argv)
//int	stdin, stdout, stderr, nargs;
{
	long	now;
	char	str[80];

	if (nargs == 2)
		clktime = 0L;
	gettime(&now);
	ascdate(now, str);
	strcat(str, "\n");
	write(argv[1], (unsigned char *)str, strlen(str));
	
	return(OK);
}
