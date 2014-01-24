/****************** DEPRECATED *********************/
//
//  Xstdio.h
//  avr-Xinu standard i/o
//
//  Created by Michael M Minor on 4/1/11.
//  Copyright 2011. All rights reserved.
//

/*
 * stdio.h
 * provides puts, printf, scanf, etc all through avr-libc.
 *
 */


/* Standard i/o for Xinu has not complied with C99
 * C99 doc: http://www.open-std.org/JTC1/SC22/WG14/www/docs/n1256.pdf
 *
 * 1)	Every stream has a Xinu file descriptor associated with it, but
 *		every device is not necessarily associated with a stream.
 * 2)	Every process is created with three text streams, standard input,
 *		standard output, and standard error.
 * 3)	Standard error is unbuffered and equivalent to kprintf:
 *			fprintf(stderr, format_string, ...); is equivalent to
 *			kprintf(format_string, ...);
 */

#ifndef _X_STDIO_H_
#define	_X_STDIO_H_ 1

#include <stdint.h>
#include <stdarg.h>

#include <stdio.h>
#undef stdin
#undef stdout
#undef stderr
#undef getc
#undef putc
#undef getchar
#undef putchar

/* libc conflicts:																	*/
/* Avoid the complications of having multiple definitions which requires a loader	*/
/* flag (-Wl,--allow-multiple-definition) and then making sure the replacement is	*/
/* resolved before libc is searched.												*/
/* Use the wrapered replacement call (see malloc.c) and the ld flag					*/
/* -Wl,--wrap,malloc,--wrap,free,--wrap,realloc                                     */

#define XINU_FDESC 
/** This macro inserts a file descriptor or unit number into a FILE stream object. */
#define file_set_fdesc(stream, u) do { (stream)->udata = (void *)u; } while(0)

/** This macro retrieves a Xinu file descriptor or device number from a FILE stream object. */
#define file_get_fdesc(stream) (int)((stream)->udata)

/*
 * stdin, stdout, stderr
 * The C99 specification states that they are macro expansions to a pointer
 */

#define STDIN 0
#define STDOUT 1
#define STDERR 2 
#define stdin  ((FILE *)(proctab[currpid].fildes[STDIN]))
#define stdout ((FILE *)(proctab[currpid].fildes[STDOUT]))
#define stderr ((FILE *)(proctab[currpid].fildes[STDERR]))


/*
 * Formatted output
 */
void _doprnt(char *, va_list, int (*)(int, FILE *), FILE *);
int _doscan(char *fmt, va_list ap, FILE *stream);

/*
 * Character input and output
 */
int putchar(int c);
int getchar(void);

/*
 * File input and output
 */
int fileno(FILE * stream);
int fclose(FILE *stream);


#endif // _X_STDIO_H_
