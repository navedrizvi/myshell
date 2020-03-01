#Naved Rizvi, CIS 3207, Project 2
CFLAGS=-g -Wall

myshell: myshell.c
	gcc $(CFLAGS) -o myshell myshell.c