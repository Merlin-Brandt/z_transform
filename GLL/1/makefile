
all: gll.o
	ar -crs libGLL.a gll.o
gll.o: gll.c gll.h
	gcc -Wall -g -c gll.c
	
install:
	cp libGLL.a /usr/local/lib/libGLL.a
	cp gll.h /usr/local/include/gll.h
	
clean:
	rm gll.o libGLL.a
