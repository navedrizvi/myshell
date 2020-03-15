#Naved Rizvi, CIS 3207, Project 2
CC = gcc
CFLAGS= -Wall

objects = myshell.o utility.o

myshell: $(objects)
	$(CC) $(CFLAGS) -o myshell $(objects)

myshell.o: myshell.c myshell.h
	$(CC) $(CFLAGS) -c myshell.c

utility.o: utility.c
	$(CC) $(CFLAGS) -c utility.c

clean :
	rm $(objects)