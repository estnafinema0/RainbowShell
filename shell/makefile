CC = gcc
CFLAGS = -Wall -g
OBJS = main.o lexer.o parser.o executor.o builtins.o colors.o

myshell: $(OBJS)
	$(CC) $(CFLAGS) -o myshell $(OBJS) -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f myshell $(OBJS)
