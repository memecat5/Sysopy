all: zad1 sender calculator

zad1: zad1.c
	gcc -g -O2 zad1.c -o zad1 -lm

sender: sender.c
	gcc -g -O2 sender.c -o sender

calculator: calculator.c
	gcc -g -O2 calculator.c -o calculator -lm

.PHONY: clean

clean:
	rm -f zad1
	rm -f calculator
	rm -f sender
