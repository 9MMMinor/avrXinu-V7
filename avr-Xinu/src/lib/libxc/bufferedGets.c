/*
 *  bufferedGets.c
 *  
 *
 *  Created by Michael Minor on 1/13/11.
 *  Copyright 2011 Michael M Minor. All rights reserved.
 *
 */
 
/*
 *---------------------------------------------------------------------------
 * open(), close(), gets() from a buffered stream
 *---------------------------------------------------------------------------
 */ 


#include <conf.h>
#include <kernel.h>
#include <name.h>
#include <io.h>

/*
 *---------------------------------------------------------------------------
 * fgets - get a string from the device (file) attached to a stream
 *---------------------------------------------------------------------------
 */
 
char *fgets(int dev, char *s, int n)
{
	register char *p = s;
	register int c;
	
	while (--n > 0 && (c = getbchar(dev)) != '\n' && c != SYSERR)
		*p++ = (char)c;
	if (c == SYSERR)
		{
		*p = '\0';
		return (char *)SYSERR;
		}
	if (c == '\n')
		*p++ = (char)c;
	*p = '\0';
	return s;
}

/*
 *------------------------------------------------------------------------
 *  fputs  --  write a null-terminated string to a device (file)
 *------------------------------------------------------------------------
 */
 
int fputs(int dev, char *s)
{
	register r;
	register c;
	int	putc();
	
	while (c = *s++)
		r = putbcharc(dev, c);
	return(r);
}


static struct stream {
	int n;
	char *bufp;
	char *buf;
} stream[NDEVS];

int getbchar(int dev)
{

	if (stream[dev].n <= 0)		/* buffer is empty */
		{
		stream[dev].n = read(dev, stream[dev].buf, BUFSIZE);
		stream[dev].bufp = stream[dev].buf;
		}
	return ( (--stream[dev].n >= 0) ? *stream[dev].bufp++&CMASK : SYSERR );
}

int putbchar(int dev, int c)
{
	
	if (stream[dev].n == BUFSIZE)
		{
		write(dev, stream[dev].buf, BUFSIZE);
		stream[dev].bufp = stream[dev].buf;
		stream[dev].n = 0;
		}
	stream[dev].n++;
	return (*stream[dev].bufp++ = (char)c);
}

/*
 *-----------------------------------------------------------------------------
 * bopen - open a file and attach it to a buffered stream, return its device
 *-----------------------------------------------------------------------------
 */
 
int bopen(char * file, char *mode)
{
	int dev;
	
	if ( (dev = open(NAMESPACE, file, mode) == SYSERR )
		return SYSERR;
	stream[dev].bufp = stream[dev].buf = (char *)getmem(BUFSIZE);
	stream[dev].n = 0;
	return dev;
}

/*
 *-----------------------------------------------------------------------------
 * bclose - close a buffered stream which is attached to a device, dev
 *-----------------------------------------------------------------------------
 */
 
int bclose(int dev)
{
	
	freemem(stream[dev].buf, BUFSIZE);
	close(dev);
}
