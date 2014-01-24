/*
 * stdio.h
 *    stdio.h -- this is the Xinu version in {Xinu}/src/sys/h
 * provides puts, printf, scanf. See also avr-Xinu.h
 *
 * 9/14/2011  -- superseeds other Xstdio.h, Xinu_stdio.h, versions
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

#include <stdarg.h>

/********************* EDIT **************************/
/* fix: this could be done more cleanly, edit path   */
/* to avr-libc, if different                         */
/********************  EDIT  *************************/
#include "/usr/local/Cellar/avr-libc/1.8.0/avr/include/stdio.h"
#undef stdin
#undef stdout
#undef stderr
#undef EOF
#undef NULL
#undef getc
#undef putc
#undef getchar
#undef putchar


/* stdio buffers */
struct __sbuf {
	char *_base;	/* unmodified buffer */
	int	_size;		/* as in ptr->_base = malloc( ptr->_size ); */
};

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

#define file_get_fdesc(stream) ((int)fdev_get_udata(stream))
#define file_set_fdesc(stream,fd) fdev_set_udata((stream), (void *)(fd))

/*
 * prototypes for stdio not in avr-libc stdio.h
 */

FILE *fopen(char *, char *);
int fileno(FILE *);
