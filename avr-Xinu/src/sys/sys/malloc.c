/*
 *  malloc.c - malloc(), free() and realloc()
 *  avr-Xinu
 *
 *  Created by Michael Minor on 3/11/10.
 *  Copyright 2010. All rights reserved.
 *
 */
#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <stddef.h>

extern int *getmem(unsigned nbytes);
extern SYSCALL freemem(void *block, unsigned size);

/*
 *------------------------------------------------------------------------
 * malloc  --  memory allocate heap storage, saving size at the head
 *------------------------------------------------------------------------
 */

void *
__wrap_malloc(size_t len)
{
	char *cp;
	WORD *original;
	
	len = (size_t)roundmb(len) + sizeof(size_t);
	cp = (char *)getmem(len);
	original = (WORD *)cp;
	*original = len;
	cp += sizeof(size_t);
	return (void *)cp;
}

/*
 *------------------------------------------------------------------------
 * free -- free len bytes allocated with malloc
 *------------------------------------------------------------------------
 */

void
__wrap_free(void *p)
{
	char *cpnew;
	
	/* ISO C: free(NULL) must be a no-op */
	if (p == NULL)
		return;
	
	cpnew = p;
	cpnew -= sizeof(size_t);
	freemem(cpnew,*(int *)cpnew);
	return;
}

void *
__wrap_realloc(void * ptr, size_t size)
{
	return NULL;
}


