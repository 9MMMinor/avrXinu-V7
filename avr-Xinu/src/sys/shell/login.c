/* login.c - login */

#include <avr-Xinu.h>
#include <shell.h>

extern int mark(int *loc);
extern SYSCALL getutim(long *time);

/*
 *------------------------------------------------------------------------
 *  login  -  log user onto system
 *------------------------------------------------------------------------
 */
 
int login(int dev)
{
	int	len;
	FILE *stream = stdout;

	Shl.shused = FALSE;
	Shl.shuser[0] = NULLCH;
	for (getname(Shl.shmach) ; TRUE ; ) {
		fprintf(stream,"\n\n%s - Xinu\n\nlogin: ",
			Shl.shmach);
		while ( (len=read(file_get_fdesc(stream),(unsigned char *)Shl.shbuf,SHBUFLEN))==0 || len==1)
			fprintf(stream, "login: ");
		if (len == EOF) {
			read(file_get_fdesc(stream), (unsigned char *)Shl.shbuf, 0);
			Shl.shused = FALSE;
			continue;
		}
		Shl.shbuf[len-1] = NULLCH;
		strncpy(Shl.shuser, Shl.shbuf, SHNAMLEN-1);
		Shl.shused = TRUE;
		getutim(&Shl.shlogon);
		mark(Shl.shmark);
		fprintf(stream,"\n%s\n\n",
		"      Welcome to Xinu (type ? for help)" );
		getutim(&Shl.shlast);
		return(OK);
	}
}
