CFLAGS=-g -Wall -O2

static: main.c lib_collatz.c
	gcc $(CFLAGS) -c lib_collatz.c -o lib_collatz.o
	gcc $(CFLAGS) -c main.c -o main.o
	ar rsc lib_collatz.a lib_collatz.o
	gcc $(CFLAGS) main.o -o main.exe -L. lib_collatz.a
	del *.o
	del *.a

dynamic: main.c lib_collatz.c
	gcc $(CFLAGS) -c lib_collatz.c -o lib_collatz.o
	gcc $(CFLAGS) -c main.c -o main.o
	gcc $(CFLAGS) -shared -fPIC -o lib_collatz.so lib_collatz.o
	gcc $(CFLAGS) main.o -o main.exe -L. lib_collatz.so
	del *.o

dll: main.c lib_collatz.c
	gcc $(CFLAGS) -c lib_collatz.c -o lib_collatz.o
	gcc $(CFLAGS) -c main.c -o main.o
	gcc $(CFLAGS) -shared -fPIC -o lib_collatz.so lib_collatz.o
	gcc -Wall main.c -o main.exe -L. -ldl -D DLL
	del *.o

.PHONY: clean

clean:
	del *.o
	del *.a
	del *.so
	del *.exe