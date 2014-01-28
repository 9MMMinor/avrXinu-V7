/*
 *  fopen.c
 *  
 *
 *  Created by Michael Minor on 1/23/11.
 *  Copyright 2011 Michael M Minor. All rights reserved.
 *
 */
 
#include <avr-Xinu.h>
#include <stdlib.h>		/* for calloc() */
#include <file.h>

FILE *fdopen(int fdes, char *mode);
int Xinu_putc(char ch, FILE * stream);
int Xinu_getc(FILE * stream);
FILE *__wrap_fdevopen(int (*put)(char, FILE *), int (*get)(FILE *));

/*
 *-----------------------------------------------------------------
 * fopen - open a file and return a stream pointer
 *-----------------------------------------------------------------
 */

#ifdef NAMESPACE
FILE *fopen(char *name, char *mode)
{
	int fildes;
	
	if ( (fildes = open(NAMESPACE, name, mode)) == SYSERR)
		return ((FILE *)SYSERR);
	return (fdopen(fildes, mode));
}
#endif

/*
 *-----------------------------------------------------------------
 * fdopen - return a stream pointer for the descriptor, fdes
 *-----------------------------------------------------------------
 */

FILE *fdopen(int fdes, char *mode)
{
	FILE *streamPtr;
	int flmode;
	
	flmode = ckmode(mode);
	if (flmode & FLRW)
		streamPtr = fdevopen(Xinu_putc, Xinu_getc); /* sets stdout, stdin, stderr */
	else if (flmode & FLREAD)
		streamPtr = fdevopen(0, Xinu_getc); /* sets stdin */
	else if (flmode & FLWRITE)
		streamPtr = fdevopen(Xinu_putc, 0); /* sets stdout, stderr */
	else
		return ((FILE *)SYSERR);
	
    file_set_fdesc(streamPtr, fdes); /* Set fdesc in stream */
	return (streamPtr);
}

/*
 *-------------------------------------------------------------------
 * fileno - return file descriptor for the stream
 *-------------------------------------------------------------------
 */

int fileno(FILE * stream)
{
	
	return ( file_get_fdesc(stream) );
}

/* Copyright (c) 2002,2005, 2007 Joerg Wunsch
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in
 the documentation and/or other materials provided with the
 distribution.
 * Neither the name of the copyright holders nor the names of
 contributors may be used to endorse or promote products derived
 from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

/* $Id: fdevopen.c,v 1.7.2.1 2009/04/01 23:11:57 arcanum Exp $ */

/*
 *-------------------------------------------------------------------
 * fdevopen - allocate a stream and return a pointer to it.
 *      Replaces fdevopen() in avr-libc.
 *-------------------------------------------------------------------
 */

FILE *__wrap_fdevopen(int (*put)(char, FILE *), int (*get)(FILE *))
{
	FILE *s;
	struct pentry *pptr = &proctab[currpid];
    unsigned char flags;
	
	if (put == 0 && get == 0)
		return (FILE *)SYSERR;
	
	if ((s = calloc(1, sizeof(FILE))) == 0)
		return (FILE *)SYSERR;
	
	flags = __SMALLOC;  /* we should not pretend to have knowledge of the FILE struct */
                        /* Since every Xinu stream except kprint_out is malloc(ed),  */
                        /* this flag is unnecessary except in the case of user initialized */
                        /* FILE structure using the FDEV_SETUP_STREAM macro */
	
	if (get != 0) {
		flags |= _FDEV_SETUP_READ;
		if (pptr->fildes[STDIN] == (void *)FDFREE)
			pptr->fildes[STDIN] = (void *)s;
	}
	
	if (put != 0) {
		flags |= _FDEV_SETUP_WRITE;
		if (pptr->fildes[STDOUT] == (void *)FDFREE)
			pptr->fildes[STDOUT] = (void *)s;
		if (pptr->fildes[STDERR] == (void *)FDFREE)
			pptr->fildes[STDERR] = (void *)s;
	}
    fdev_setup_stream(s,put,get,flags);
	
	return s;
}

