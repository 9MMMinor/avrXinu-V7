#
#  Make Xinu Manual Pages - only FILES change between man[01234]
#

NROFF = groff

%.pdf : %.4
		$(NROFF) -mdoc $< | pstopdf -i -o $@
#		$(NROFF) -mdoc -Thtml $< >$*.html

FILES	=	Intro4.pdf	\
			ether.pdf	\
			internet.pdf	\
			namespace.pdf	\
			rfilsys.pdf	\
			tty.pdf

all:		$(FILES)

install:
		@echo no installation possible - use "'make print'"

clean:
		rm -rf *.html *.pdf *.ps

