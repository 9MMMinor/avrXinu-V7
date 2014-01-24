/*
 *  avr-Xinu.h
 *  
 *
 *  Created by Michael Minor on 9/7/10.
 *  Copyright 2010 Michael M Minor. All rights reserved.
 *
 */
 
/* xinu */
#include <stdio.h>		/* REPLACEMENT *printf, *scanf, etc */
#include <conf.h> 
#include <kernel.h>
#include <proc.h>

/* avr-libc */
#include <stdint.h>		/* int16_t, uint16_t, etc */
#include <string.h>		/* memcpy, memcmp, etc */

#undef getc
#undef putc
#undef getchar
#undef putchar

/* libc conflicts:																	*/
/* Avoid the complications of having multiple definitions which requires a loader	*/
/* flag (-Wl,--allow-multiple-definition) and then making sure the replacement is	*/
/* resolved before libc is searched.												*/
/* Use the wrapered replacement call (see malloc.c) and the ld flag					*/
/* -Wl,--wrap,malloc,--wrap,free,--wrap,realloc,--wrap,fdevopen						*/

/* Xinu System Calls */
extern SYSCALL chprio(int pid, int newprio);
extern int ckmode(char *mode);
extern SYSCALL close(int dev);
extern SYSCALL control(int dev, int function, void *arg1, void *arg2);
extern SYSCALL create(int (*func)(), int size, int pri, char *name, int nargs, ...);
extern SYSCALL _getc(int descrp);
extern WORD *getmem(unsigned nbytes);
extern SYSCALL getname(char *nam);
extern SYSCALL freemem(void *block, unsigned size);
extern void *__wrap_malloc(size_t len);
extern void *malloc(size_t len);
extern void __wrap_free(void *p);
extern void free(void *p);
extern int getpid(void);
extern int getprio(int pid);
extern SYSCALL kill(int pid);
extern SYSCALL open(int descrp, char *name, char *mode);
extern SYSCALL pcount(int portid);
extern SYSCALL pcreate(int count);
extern SYSCALL pdelete(int portid, int (*dispose)());
extern SYSCALL preceive(int portid);
extern SYSCALL preset(int portid, int (*dispose)());
extern SYSCALL psend(int portid, WORD msg);
extern SYSCALL _putc(int descrp, char ch);
extern SYSCALL read(int descrp, unsigned char *buff, int count);
extern SYSCALL receive(void);
extern SYSCALL recvclr(void);
extern SYSCALL recvtim(int maxwait);
extern SYSCALL resume(int pid);
extern SYSCALL scount(int sem);
extern SYSCALL screate(int count);
extern SYSCALL sdelete(int sem);
extern int seek(int descrp, long pos);
extern SYSCALL send(int pid, WORD msg);
extern SYSCALL sendf(int pid, int msg);
extern SYSCALL signal(int sem);
extern SYSCALL signaln(int sem, int count);
extern SYSCALL sleep(int n);
extern SYSCALL sleep10(int n);
extern SYSCALL sreset(int sem, int count);
extern SYSCALL suspend(int pid);
extern SYSCALL wait(int sem);
extern SYSCALL write(int descrp, unsigned char *buff, int count);
extern void panic (char *msg);
extern void abort(void);
extern int blkequ(register char *a, register char *b, register int16_t);

/* In-line I/O procedures (replace avr-libc functions) */
#define	getchar()	_getc(CONSOLE)
#define	putchar(ch)	_putc(CONSOLE,(ch))
#define getc() fgetc(stdin)
#define putc(ch) fputc((ch),stdout)


/* avr-libc */
#define blkcopy(to,from,len) memcpy((to), (from), (len))
