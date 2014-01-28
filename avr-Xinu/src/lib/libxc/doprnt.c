/* doprnt.c - _doprnt */

/* adapted by S. Salisbury, Purdue U.	*/
/* adapted by M. Minor, Jan 2011, for avr-Xinu */
/* DEPRICATED in avr-Xinu **********************/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <avr/pgmspace.h>

#define	MAXSTR	80

/* for *fmt++ -> GETBYTE(stream->flags,__SPGM,fmt) */
# define GETBYTE(flag, mask, pnt)	({	\
unsigned char __c;				\
__c = ((flag) & (mask))			\
? pgm_read_byte(pnt) : *pnt;		\
pnt++;					\
__c;					\
})

/* for *fmt -> CHKBYTE(stream->flags,__SPGM,fmt) */
# define CHKBYTE(flag, mask, pnt)	({	\
unsigned char __c;				\
__c = ((flag) & (mask))			\
? pgm_read_byte(pnt) : *pnt;		\
__c;					\
})


/*
 *------------------------------------------------------------------------
 *  _doprnt --  format and write output using 'func' to write characters
 *------------------------------------------------------------------------
 */
 
void _doprnt(char *fmt, va_list ap, int (*func)(int, FILE *), FILE *stream)
	{
	int	c;
	int	i;
	int	f;		/* The format character	(comes after %)	*/
	char	*str;		/* Running pointer in string		*/
	char	string[20];	/* The string str points to this output	*/
						/*  from number conversion		*/
	int	length;		/* Length of string "str"		*/
	char fill;		/* Fill	character (' ' or '0')		*/
	int	leftjust;	/* 0 = right-justified,	else left-just.	*/
	int	longflag;	/* != 0	for long numerics - not	used	*/
	int	fmax,fmin;	/* Field specifications	% MIN .	MAX s	*/
	int	leading;	/* No. of leading/trailing fill	chars.	*/

	for(;;) {
		/* Echo characters until '%' or end of fmt string */
		while( (c = GETBYTE(stream->flags,__SPGM,fmt)) != '%' ) {
			if( c == '\0' )
				return;
			(*func)(c,stream);
		}
		/* Echo "...%%..." as '%' */
		if( CHKBYTE(stream->flags,__SPGM,fmt) == '%' ) {
			(*func)(GETBYTE(stream->flags,__SPGM,fmt),stream);
			continue;
		}
		/* Check for "%-..." == Left-justified output */
		if ((leftjust = ((CHKBYTE(stream->flags,__SPGM,fmt)=='-')) ? 1 : 0) )
			fmt++;
		/* Allow for zero-filled numeric outputs ("%0...") */
		fill = (CHKBYTE(stream->flags,__SPGM,fmt)=='0') ? GETBYTE(stream->flags,__SPGM,fmt) : ' ';
		/* Allow for minimum field width specifier for %d,u,x,o,c,s*/
		/* Also allow %* for variable width (%0* as well)	*/
		fmin = 0;
		if( CHKBYTE(stream->flags,__SPGM,fmt) == '*' ) {
			fmin = va_arg(ap, int);
			++fmt;
		}
		else while( '0' <= CHKBYTE(stream->flags,__SPGM,fmt) && CHKBYTE(stream->flags,__SPGM,fmt) <= '9' ) {
			fmin = fmin * 10 + GETBYTE(stream->flags,__SPGM,fmt) - '0';
		}
		/* Allow for maximum string width for %s */
		fmax = 0;
		if( CHKBYTE(stream->flags,__SPGM,fmt) == '.' ) {
			fmt++;
			if( CHKBYTE(stream->flags,__SPGM,fmt) == '*' ) {
				fmax = va_arg(ap, int);
				++fmt;
			}
		else while( '0' <= CHKBYTE(stream->flags,__SPGM,fmt) && CHKBYTE(stream->flags,__SPGM,fmt) <= '9' ) {
			fmax = fmax * 10 + GETBYTE(stream->flags,__SPGM,fmt) - '0';
			}
		}
		/* Check for the 'l' option to force long numeric */
		if( (longflag = ((CHKBYTE(stream->flags,__SPGM,fmt) == 'l')) ? 1 : 0) )
			fmt++;
		str = string;
		*str = '\0';
		if( (f= GETBYTE(stream->flags,__SPGM,fmt)) == '\0' ) {
			(*func)('%',stream);
			return;
		}

		switch( f ) {
		
		case 'c' :
			string[0] = va_arg(ap, int);
			string[1] = '\0';
			fmax = 0;
			fill = ' ';
			break;

		case 's' :
			str = va_arg(ap, char *);
			fill = ' ';
			break;

		case 'd' :
		case 'i' :
			if (longflag)
				ltoa(va_arg(ap, long), str, 10);
			else
				itoa(va_arg(ap, int), str, 10);
			break;
			
		case 'u':
			if( longflag )
				ultoa(va_arg(ap, unsigned long), str, 10);
			else
				utoa(va_arg(ap, unsigned int), str, 10);
			fmax = 0;
			break;

		case 'o' :
			if ( longflag )
				ultoa(va_arg(ap, unsigned long), str, 8);
			else
				utoa(va_arg(ap, unsigned int), str, 8);
			fmax = 0;
			break;

		case 'p' :
			*str++ = '0';
			*str++ = 'x';
		case 'X' :
		case 'x' :
			if( longflag )
				ultoa(va_arg(ap, unsigned long), str, 16);
			else
				utoa(va_arg(ap, unsigned int), str, 16);
			fmax = 0;
			str = string;
			break;

		default :
			(*func)(f,stream);
			break;
		}

		for(length = 0; str[length] != '\0'; length++)
			;
		if ( fmin > MAXSTR || fmin < 0 )
			fmin = 0;
		if ( fmax > MAXSTR || fmax < 0 )
			fmax = 0;
		leading = 0;
		if ( fmax != 0 || fmin != 0 ) {
			if ( fmax != 0 )
				if ( length > fmax )
					length = fmax;
			if ( fmin != 0 )
				leading = fmin - length;
		}
		if( leftjust == 0 )
			for( i = 0; i < leading; i++ )
				(*func)(fill,stream);
		for( i = 0 ; i < length ; i++ )
			{
			if ( f == 'X' && ( 'a' <= str[i] && str[i] <= 'f') )
				str[i] += 'A' - 'a';
			(*func)(str[i],stream);
			}
		if ( leftjust != 0 )
			for( i = 0; i < leading; i++ )
				(*func)(fill,stream);
	}
}
