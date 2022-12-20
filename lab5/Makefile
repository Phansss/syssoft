# the make automatic variable ‘$<’ will represent the source file wherever it is
# When the target has multiple dependencies, using an explicit ‘$(srcdir)’ is the easiest way to make the rule work well
# building a program in ordinary circumstances should not modify the source directory in any way. 
# IMPORTANT:
# The configure script and the Makefile rules for building and installation should not use any utilities directly except these: 
#	awk cat cmp cp diff echo egrep expr false grep install-info ln ls
#	mkdir mv printf pwd rm rmdir sed sleep sort tar test touch tr true

# The Makefile rules for building and installation can also use compilers and related programs
#  but should do so via make variables so that the user can substitute alternatives.
#	ar bison cc flex install ld ldconfig lex make makeinfo ranlib texi2dvi yacc
# Use the following make variables to run those programs: 
#	$(AR) $(BISON) $(CC) $(FLEX) $(INSTALL) $(LD) $(LDCONFIG) $(LEX) $(MAKE) $(MAKEINFO) $(RANLIB) $(TEXI2DVI) $(YACC)
# Additional utilities that can be used via Make variables are: 
#	chgrp chmod chown mknod
#

SHELL = /bin/sh
.SUFFIXES:
.SUFFIXES: .c .o
LDCONFIG = ldconfig
FLAGS = -std=c11 -Werror -lm -DSET_MAX_TEMP=27.96 -DSET_MIN_TEMP=14.01 $(shell pkg-config --cflags --libs check)

ex3: 
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for DATAMGR TEST ***'
	@echo -e '*******************************'
	mkdir -p build
	gcc datamgr.c test_main.c -ldplist  -o build/datamgr_test $(FLAGS)
	@echo -e '\n*************************'
	@echo -e '*** Running DATAMGR TEST ***'
	@echo -e '*************************'
	./build/datamgr_test

test_debug :
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	mkdir -p build
	gcc -g datamgr.c test_main.c -ldplist  -o build/datamgr_test_debug $(FLAGS)
	@echo -e '\n*************************'
	@echo -e '*** Running UNIT TEST ***'
	@echo -e '*************************'
	export CK_FORK=no ; gdb -q -tui ./build/datamgr_test_debug

ex2: libdplist.so
	gcc ex2/main.c -ldplist -o build/ex2_shared
	@echo -e '\n***********List Dynamic Dependencies (ldd)***********'
	ldd build/ex2_shared
	@echo -e '\n***********Program Output (ex2: shared lib) ***********'
	./build/ex2_shared

ex1: libdplist.a
	gcc ex1/main.c lib_static/libdplist.a -o build/ex1_static $(FLAGS)
	@echo -e '\n***********Program Output (ex1: static lib) ***********'

	./build/ex1_static

libdplist.so: lib/dplist.o
	cc -shared -o libdplist.so lib/dplist.o
	sudo cp libdplist.so /usr/lib
	sudo $(LDCONFIG)
	rm lib/dplist.o

shared_dplist.o: 
	gcc lib/dplist.c -o lib/dplist.o -fPIC -c 
	
# the files for ex3 will be ziped and are then ready to
# be submitted to labtools.groept.be
zip:
	zip lab5_ex3.zip datamgr.c datamgr.h config.h lib/dplist.c lib/dplist.h


libdplist.a: lib_static/dplist.o
	ar rs lib_static/libdplist.a lib_static/dplist.o
	rm lib_static/dplist.o
static_dplist.o: 
	gcc -c lib_static/dplist.c -o lib_static/dplist.o


