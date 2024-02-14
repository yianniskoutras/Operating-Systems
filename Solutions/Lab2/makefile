CC = gcc
CFLAGS = -Wall -Wextra -pedantic

all: parent_test child

parent_test: parent_test.o
	$(CC) $(CFLAGS) -o parent_test parent_test.o

parent_test.o: parent_test.c
	$(CC) $(CFLAGS) -c parent_test.c

child: child.o
	$(CC) $(CFLAGS) -o child child.o

child.o: child.c
	$(CC) $(CFLAGS) -c child.c

clean:
	rm -f parent_test child parent_test.o parent_test.o