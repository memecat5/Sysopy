CC= gcc
CFLAGS=-g -Wall
all: countdown.exe

countdown.exe: countdown.c
	$(CC) $(CFLAGS) countdown.c -o countdown.exe

.PHONY: clean

clean:
	del countdown.exe