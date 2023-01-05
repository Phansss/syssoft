SHELL = /bin/sh
.SUFFIXES:
.SUFFIXES: .c .o
LDCONFIG = ldconfig
FLAGS = -std=c11 -Werror -lm $(shell pkg-config --cflags --libs check)

main:
	gcc -Wall main.c sbuffer.c -o ./build/main_sbuff -ldplist $(FLAGS)
	./build/main_sbuff

main_debug:
	gcc -g -Wall main.c sbuffer.c lib/dplist.c  -o ./build/main_sbuff_debug $(FLAGS)
	export CK_FORK=no ; gdb -q -tui ./build/main_sbuff_debug

libdplist.so: lib/dplist.o
	cc -shared -o libdplist.so lib/dplist.o
	sudo cp libdplist.so /usr/lib
	sudo $(LDCONFIG)
	rm lib/dplist.o

shared_dplist.o:
	gcc lib/dplist.c -o lib/dplist.o -fPIC -c
