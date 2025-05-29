all: zad1 sender catcher

zad1: zad1.c
	gcc -g zad1.c -o zad1

sender: sender.c
	gcc -g sender.c -o sender

catcher: catcher.c
	gcc -g catcher.c -o catcher

.PHONY: clean

clean:
	rm -f zad1
	rm -f sender
	rm -f catcher
