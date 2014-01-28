/* doscan.c - doscan */

/* Character look-ahead could be implemented as: ch=control(unit,TTY_IOC_NEXTC) */

/* adapted by M. Minor, Jan 2011, for avr-Xinu */
/* DEPRICATED in avr-Xinu **********************/

#include <kernel.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>


#define	SPC	01
#define	STP	02
#define EOF -2

#define	typeSHORT	0
#define	typeINT		1
#define	typeLONG	2

static int _innum(void *, int, int, int, FILE *, int *);
static int _instr(char *, int, int, FILE *, int *);
static char *_getccl(char *);

char _sctab[128] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, SPC, SPC, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    SPC, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
};

/*
 *------------------------------------------------------------------------
 *  _doscan  --  scan and recognize input according to a format
 *------------------------------------------------------------------------
 */

int _doscan(char *fmt, va_list ap, FILE *stream)
{
	int ch;
    int nmatch, len, ch1;
    int fileended, size;
	void *ptr;

    nmatch = 0;
    fileended = 0;
    for (;;)
		{
        switch (ch = *fmt++)
			{
        case '\0':
            return (nmatch);
			
        case '%':
            if ((ch = *fmt++) == '%')
                goto def;
            ptr = 0;
            if (ch != '*')
                ptr = va_arg(ap, void *);
            else
                ch = *fmt++;
            len = 0;
            size = typeINT;
            while (isdigit(ch))
				{
                len = len * 10 + ch - '0';
                ch = *fmt++;
				}
            if (len == 0)
                len = 30000;
            if (ch == 'l')
				{
                ch = *fmt++;
                size = typeLONG;
				}
            else if (ch == 'h')
				{
                size = typeSHORT;
                ch = *fmt++;
            	}
            else if (ch == '[')
                fmt = _getccl(fmt);
            if (isupper(ch))
            	{
                ch = tolower(ch);
                size = typeLONG;
            	}
            if (ch == '\0')
                return (-1);
            if ( _innum(ptr, ch, len, size, stream, &fileended) && ptr)
				{
                nmatch++;
				}
            if (fileended)
                return (nmatch ? nmatch : -1);
            break;

        case ' ':
        case '\n':
        case '\t':
            while ( (ch1 = fgetc(stream) ) == ' ' || ch1 == '\t' || ch1 == '\n')
            	;
            if (ch1 != EOF)
                ungetc(ch1, stream);
            break;

        default:
		def:
            ch1 = fgetc(stream);
            if (ch1 != ch)
            	{
                if (ch1 == EOF)
                	{
                    return (-1);
                	}
                ungetc(ch1, stream);
                return nmatch;
            	}
        	}
    	}
}

static int _innum(void *ptr, int type, int len, int size, FILE *stream, int *eofptr)
{
///    extern double atof(char *p);
    register char *np;
    char numbuf[64];
    register char c, base;
    int expseen, negflg, c1, ndigit;
    long lcval;

    if (type == 'c' || type == 's' || type == '[')
        return (_instr(ptr ? (char *)ptr : NULL, type, len, stream, eofptr));
    lcval = 0;
    ndigit = 0;
    base = 10;
    if (type == 'o')
        base = 8;
    else if (type == 'x')
        base = 16;
    np = numbuf;
    expseen = 0;
    negflg = 0;
    while ((c = fgetc(stream)) == ' ' || c == '\t' || c == '\n')
		;
    if (c == '-')
    	{
        negflg++;
        *np++ = c;
        c = fgetc(stream);
        len--;
    	}
    else if (c == '+')
    	{
        len--;
        c = fgetc(stream);
    	}

    for (; --len >= 0; *np++ = c, c = fgetc(stream))
    	{
        if (isdigit(c)
				|| (base == 16 && (('a' <= c && c <= 'f') || ('A' <= c && c <= 'F'))))
        	{
            ndigit++;
            if (base == 8)
                lcval <<= 3;
            else if (base == 10)
                lcval = ((lcval << 2) + lcval) << 1;
            else
                lcval <<= 4;
            c1 = c;
            if ('0' <= c && c <= '9')
                c -= '0';
            else if ('a' <= c && c <= 'f')
                c -= 'a' - 10;
            else
                c -= 'A' - 10;
            lcval += c;
            c = c1;
            continue;
        	}
        else
            break;
    	}
    if (negflg)
        lcval = -lcval;
    if (c != EOF)
		{
        ungetc(c, stream);
        *eofptr = 0;
		}
    else
        *eofptr = 1;
    if (ptr == NULL || np == numbuf)
        return 0;
    *np++ = 0;
    switch (size)
    	{
    case typeSHORT:
        *(short *)ptr = lcval;
        break;

    case typeINT:
        *(int *)ptr = lcval;
        break;

    case typeLONG:
        *(long *)ptr = lcval;
        break;
    	}
    return (1);
}

static int _instr(char *ptr, int type, int len, FILE *stream, int *eofptr)
{
    unsigned int ch;
    char *optr;
    int ignstp;

    *eofptr = 0;
    optr = ptr;
    if (type == 'c' && len == 30000)
        len = 1;
    ignstp = 0;
    if (type == 's')
        ignstp = SPC;
    while (_sctab[ch = fgetc(stream)] & ignstp)
    	{
        if (ch == EOF)
            break;
    	}
    ignstp = SPC;
    if (type == 'c')
        ignstp = 0;
    else if (type == '[')
        ignstp = STP;
    while (ch != EOF && (_sctab[ch] & ignstp) == 0)
    	{
        if (ptr)
            *ptr++ = ch;
        if (--len <= 0)
            break;
        ch = fgetc(stream);
    	}
    if (ch != EOF)
    	{
        if (len > 0)
            ungetc(ch, stream);
        *eofptr = 0;
    	}
    else
        *eofptr = 1;
    if (ptr && ptr != optr)
		{
        if (type != 'c')
            *ptr++ = '\0';
        return (1);
		}
    return (0);
}

static char *_getccl(char *s)
{
    int c, t;

    t = 0;
    if (*s == '^')
		{
        t++;
        s++;
    	}
    for (c = 0; c < 128; c++)
        if (t)
            _sctab[c] &= ~STP;
        else
            _sctab[c] |= STP;
    while (((c = *s++) & 0177) != ']')
    	{
        if (t)
            _sctab[c++] |= STP;
        else
            _sctab[c++] &= ~STP;
        if (c == 0)
            return (--s);
    	}
    return s;
}
