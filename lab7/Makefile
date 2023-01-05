SHELL = /bin/sh
.SUFFIXES:
.SUFFIXES: .c .o
LDCONFIG = ldconfig
FLAGS = -std=c11 -Werror -lm $(shell pkg-config --cflags --libs check)

sensor_node:
	mkdir -p build
	gcc -Wall sensor_node.c -o ./build/sensor_node -ltcpsock -ldplist $(FLAGS)

s1:
	./build/sensor_node 1 5 127.0.0.1 1234

s2:
	./build/sensor_node 2 10 127.0.0.1 1234

s3:
	./build/sensor_node 3 3 127.0.0.1 1234

main:
	gcc -Wall main.c connmgr.c -o ./build/main_conn -ltcpsock -ldplist $(FLAGS)
	./build/main_conn

main_debug:
	gcc -g -Wall main.c connmgr.c lib/dplist.c lib/tcpsock.c -o ./build/main_conn_debug $(FLAGS)
	export CK_FORK=no ; gdb -q -tui ./build/main_conn_debug

main_server:
	mkdir -p build
	gcc -Wall main_server.c connmgr.c -o ./build/main_server -ltcpsock -ldplist  $(FLAGS)


main_server_debug:
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	mkdir -p build
	gcc -g -Wall main_server.c connmgr.c  -o ./build/main_server_debug -ltcpsock -ldplist $(FLAGS)
	@echo -e '\n*************************'
	@echo -e '*** Running UNIT TEST ***'
	@echo -e '*************************'
	export CK_FORK=no ; gdb -q -tui ./build/main_server_debug

test_server:
	gcc -Wall test_server.c -o ./build/test_server -ltcpsock $(FLAGS)
	./build/test_server

test_server_debug:
	gcc -g -Wall test_server.c -o ./build/test_server_debug -ltcpsock $(FLAGS)
	export CK_FORK=no ; gdb -q -tui ./build/test_server_debug

libtcpsock.so: lib/tcpsock.o
	cc -shared -o libtcpsock.so lib/tcpsock.o
	sudo cp libtcpsock.so /usr/lib
	sudo $(LDCONFIG)
	rm lib/tcpsock.o


shared_tcpsock.o:
	gcc lib/tcpsock.c -o lib/tcpsock.o -fPIC -c

libdplist.so: lib/dplist.o
	cc -shared -o libdplist.so lib/dplist.o
	sudo cp libdplist.so /usr/lib
	sudo $(LDCONFIG)
	rm lib/dplist.o

shared_dplist.o:
	gcc lib/dplist.c -o lib/dplist.o -fPIC -c

zip:
	zip lab7_ex2.zip connmgr.c connmgr.h config.h lib/dplist.c lib/dplist.h lib/tcpsock.c lib/tcpsock.h
