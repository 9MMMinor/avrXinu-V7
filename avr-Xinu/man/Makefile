#
#  Make All Xinu Manual Pages
#

all:
		@echo making all manual pages in man[01234]
		cd man0;make print;cd ..
		cd man1;make print;cd ..
		cd man2;make print;cd ..
		cd man3;make print;cd ..
		cd man4;make print;cd ..

install:
		@echo type "\`make print'" to make and print the entire manual

clean:
		rm -rf *.html core ,*
		cd man0;make clean;cd ..
		cd man1;make clean;cd ..
		cd man2;make clean;cd ..
		cd man3;make clean;cd ..
		cd man4;make clean;cd ..

print:		manual

listings:	manual

manual:		all
#		cat man0/*.html man1/*.html man2/*.html man3/*.html man4/*.html | \
			sed '1,2d;s/^/	/' | lpr
