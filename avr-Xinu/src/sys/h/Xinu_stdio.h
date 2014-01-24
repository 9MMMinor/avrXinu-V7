/****************** DEPRECATED *********************/
/*
 * stdio.h
 * provides puts, printf, scanf.
 *
 */
 
//#include <proc.h>

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
#ifndef _STDIO_H_
#define	_STDIO_H_ 1

#include <stdint.h>
#include <stdarg.h>

/* stdio buffers */
struct __sbuf {
	char *_base;	/* unmodified buffer */
	int	_size;		/* as in ptr->_base = malloc( ptr->_size ); */
};

//typedef struct __file {
//	int fdesc;				/* Xinu file descriptor or device number */
//	int	(*put)(int, char);	/* function to write one char to device */
//	int	(*get)(int);		/* function to read one char from device */
//	int	flags;				/* flags, see below */
//#define __SRD	0x0001		/* OK to read */
//#define __SWR	0x0002		/* OK to write */
//#define __SSTR	0x0004		/* this is an sprintf/snprintf string */
//#define __SPGM	0x0008		/* fmt string is in progmem */
//#define __SERR	0x0010		/* found error */
//#define __SEOF	0x0020		/* found EOF */
//#define __SUNGET 0x040		/* ungetc() happened */
//#define __SMALLOC 0x80		/* handle is malloc()ed */
//#define __SRW	0x0100		/* open for reading & writing */
//#define __SLBF	0x0200		/* line buffered */
//#define __SNBF	0x0400		/* unbuffered */
//#define __SMBF	0x0800		/* buf is from malloc */
//	struct __sbuf _buf;
//	char	*bufp;			/* buffer pointer - current position in (some) buffer */
//	int		n;				/* number of characters left in buffer (redundant, but useful) */
//	char *buf;		/* buffer pointer */
//	int	size;		/* size of buffer */
//	int	len;		/* characters read or written so far */
//	unsigned char unget;	/* ungetc() buffer */
//}FILE;

#define XINU_FDESC 
/** This macro inserts a file descriptor or unit number into a FILE stream object. */
#define file_set_fdesc(stream, u) do { (stream)->udata = (void *)u; } while(0)

/** This macro retrieves a Xinu file descriptor or device number from a FILE stream object. */
#define file_get_fdesc(stream) (int)((stream)->udata)

/*
 * This is an internal structure of the library that is subject to be
 * changed without warnings at any time.  Please do *never* reference
 * elements of it beyond by using the official interfaces provided.
 */
struct __file {
	char	*buf;		/* buffer pointer */
	unsigned char unget;	/* ungetc() buffer */
	uint8_t	flags;		/* flags, see below */
#define __SRD	0x0001		/* OK to read */
#define __SWR	0x0002		/* OK to write */
#define __SSTR	0x0004		/* this is an sprintf/snprintf string */
#define __SPGM	0x0008		/* fmt string is in progmem */
#define __SERR	0x0010		/* found error */
#define __SEOF	0x0020		/* found EOF */
#define __SUNGET 0x040		/* ungetc() happened */
#define __SMALLOC 0x80		/* handle is malloc()ed */
#if 0
    /* possible future extensions, will require uint16_t flags */
#define __SRW	0x0100		/* open for reading & writing */
#define __SLBF	0x0200		/* line buffered */
#define __SNBF	0x0400		/* unbuffered */
#define __SMBF	0x0800		/* buf is from malloc */
#endif
	int	size;		/* size of buffer */
	int	len;		/* characters read or written so far */
	int	(*put)(char, struct __file *);	/* function to write one char to device */
	int	(*get)(struct __file *);	/* function to read one char from device */
	void	*udata;		/* User defined and accessible data. */
};

#define FILE struct __file

//static struct stream
//{
//	int n;
//	char *bufp;
//	char *buf;
//} stream[NDEVS];


/*
 * Formatted input
 */
int fscanf(FILE *, char *, ...);
int scanf(char *, ...);
int sscanf(char *, char *, ...);
int _doscan(char *, va_list, FILE *);

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
//#define stdin (__iob[0])
//#define stdout (__iob[1])
//#define stderr (__iob[2])
#define FDEV_SETUP_STREAM(p, g, f) \
{ \
.put = p, \
.get = g, \
.flags = f, \
.udata = 0, \
}

#define _FDEV_SETUP_WRITE __SWR	/**< fdev_setup_stream() with write intent */

/*
 * Formatted output
 */
void _doprnt(char *, va_list, int (*)(int, FILE *), FILE *);
int fprintf(FILE *, const char *, ...);
int printf(const char *, ...);
int printf_P(const char *, ...);
int sprintf(char *, const char *, ...);

/*
 * Character input and output
 */
int fgetc(FILE *);
int ungetc(int, FILE *);
char *fgets(char *, int, FILE *);
int fputc(int, FILE *);
int fputs(const char *, FILE *);
int putchar(int c);
int getchar(void);

/*
 * File input and output
 */

FILE *fopen(char *, char *);
FILE *fdopen(int fdes, char *mode);
int fileno(FILE * stream);
FILE *fdevopen(int (*put)(int,char), int (*get)(int));
int fclose(FILE *stream);

#endif /* _STDIO_H_ */
