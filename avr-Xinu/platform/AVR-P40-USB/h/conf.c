/* conf.c (GENERATED FILE; DO NOT EDIT) */

/* device independent I/O switch */

struct	devsw	devtab[NDEVS] = {

/*  Format of entries is:
device-number, device-name,
init, open, close,
read, write, seek,
getc, putc, cntl,
device-csr-address, iint-handler, oint-handler,
control-block, minor-device,
*/

/*  CONSOLE  is tty  */

{0, "CONSOLE",
(void *)ttyinit, (void *)ttyopen, (void *)ionull,
(void *)ttyread, (void *)ttywrite, (void *)ioerr,
(void *)ttygetc, (void *)ttyputc, (void *)ttycntl,
(void *)0x00, (void *)ttyiin, (void *)ttyoin,
NULLPTR, 0}
	};
