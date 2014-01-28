#
#  Make Xinu Manual Pages - only FILES change between man[0123]
#


DIR	=	../..
MACROS	=	$(DIR)/lib/tmac.an
NROFF = groff -Thtml -mandoc

.SUFFIXES:	.ms .html .1 .2 .3 .4

.ms.html:
		$(NROFF) -ms $< > ,$* ; mv ,$* $*.html

.2.html:
		$(NROFF) $(MACROS) $< > ,$* ; mv ,$* $*.2.html

.3.html:
		$(NROFF) $(MACROS) $< > ,$* ; mv ,$* $*.3.html


FILES	=	$(MACROS)	\
		Notes.html

all:		$(FILES)

install:
		@echo no installation possible - use "'make print'"

clean:
		rm -rf *.html core ,*

print:		listings

listings:	$(FILES)
#		cat *.html | sed '1,2d;s/^/	/' | lpr
