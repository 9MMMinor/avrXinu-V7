#
#  Make Xinu Manual Pages - only FILES change between man[01234]
#

NROFF = groff

%.pdf : %.2
		$(NROFF) -mdoc $< | pstopdf -i -o $@
#		$(NROFF) -mdoc -Thtml $< >$*.html

FILES	= 	\
		access.pdf	\
		chprio.pdf	\
		close.pdf	\
		control.pdf	\
		create.pdf	\
		freebuf.pdf	\
		freemem.pdf	\
		freestk.pdf	\
		getaddr.pdf	\
		getbuf.pdf	\
		getc.pdf	\
		getmem.pdf	\
		getname.pdf	\
		getnet.pdf	\
		getpid.pdf	\
		getprio.pdf	\
		gettime.pdf	\
		getutim.pdf	\
		ip2name.pdf	\
		kill.pdf	\
		mark.pdf	\
		mkpool.pdf	\
		mount.pdf	\
		nammap.pdf	\
		namrepl.pdf	\
		open.pdf	\
		panic.pdf	\
		pcount.pdf	\
		pcreate.pdf	\
		pdelete.pdf	\
		pinit.pdf	\
		preceive.pdf	\
		preset.pdf	\
		psend.pdf	\
		putc.pdf	\
		read.pdf	\
		receive.pdf	\
		recvclr.pdf	\
		recvtim.pdf	\
		remove.pdf	\
		resume.pdf	\
		scount.pdf	\
		screate.pdf	\
		sdelete.pdf	\
		seek.pdf	\
		send.pdf	\
		sendf.pdf	\
		setdev.pdf	\
		setnok.pdf	\
		signal.pdf	\
		sleep.pdf	\
		sreset.pdf	\
		suspend.pdf	\
		unmount.pdf	\
		unsleep.pdf	\
		wait.pdf	\
		write.pdf

all:		$(FILES)

install:
		@echo no installation possible - use "'make print'"

clean:
		rm -rf *.pdf core ,* *.html
