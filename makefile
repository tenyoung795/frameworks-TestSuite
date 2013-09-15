#!/usr/bin/env make -f

CC = g++
STANDARD = c++0x
OPTIONS = -std=$(STANDARD) -pthread -Wall

compile: TestSuite.cpp TestSuite.h
	$(CC) $(OPTIONS) -c TestSuite.cpp

libtestsuite.a: TestSuite.o
	ar -cvq libtestsuite.a TestSuite.o

install: libtestsuite.a
	cp libtestsuite.a /usr/lib/
	cp TestSuite.h /usr/include/

install_local: libtestsuite.a
	cp libtestsuite.a /usr/local/lib/
	cp TestSuite.h /usr/local/include/

clean:
	rm -f TestSuite.o libtestsuite.a *~ *.swp

