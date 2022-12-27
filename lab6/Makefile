# Create your own targets that compile the application

FLAGS = -std=c11 -Werror -lm $(shell pkg-config --cflags --libs check)



ex2:
	gcc -Wall main.c sensor_db.c -o exercise2 -lsqlite3 $(FLAGS)
	./exercise2

test_debug:
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	mkdir -p build
	gcc -g -Wall main.c sensor_db.c -o ./build/exercise2 -lsqlite3 $(FLAGS)
	@echo -e '\n*************************'
	@echo -e '*** Running UNIT TEST ***'
	@echo -e '*************************'
	export CK_FORK=no ; gdb -q -tui ./build/exercise2


ex2-alt:
	gcc -Wall main.c sensor_db.c sqlite3.c -pthread -ldl

# the files for ex2 will be ziped and are then ready to
# be submitted to labtools.groept.be
zip:
	zip lab6_ex2.zip sensor_db.c config.h
