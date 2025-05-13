CC= gcc
CFLAGS=-g -Wall -o
all: countdown.exe

countdown.exe: countdown.c
	$(CC) countdown.c $(CFLAGS) countdown.exe

.PHONY: clean

clean:
	del countdown.exe