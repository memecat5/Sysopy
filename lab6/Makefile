all: zad1 sender calculator

zad1: zad1.c
	gcc -g zad1.c -o zad1 -lm

sender: sender.c
	gcc -g sender.c -o sender

calculator: calculator.c
	gcc -g calculator.c -o calculator -lm

.PHONY: clean

clean:
	rm -f zad1
	rm -f calculator
	rm -f sender
