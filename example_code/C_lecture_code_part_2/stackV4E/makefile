EXE = a.out
OBJS = main.o stack.o 
CC = gcc 
CFLAGS = -Wall -Werror -c
LFLAGS = -Wall -Werror

$(EXE): $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $(EXE)

main.o: main.c stack.h
	$(CC) $(CFLAGS) main.c

stack.o: stack.c stack.h
	$(CC) $(CFLAGS) stack.c

clean:
	rm *.o $(EXE)
