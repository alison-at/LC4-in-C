all: trace

trace: trace2.c LC4.o loader.o
	clang -g -o trace trace2.c LC4.o loader.o

LC4.o: LC4.c
	clang -g -c LC4.c

loader.o: loader.c
	clang -g -c loader.c

clean:
	rm -rf *.o

clobber: clean
	rm -rf trace
