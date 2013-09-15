#!/usr/bin/env make -f

CC = g++
STANDARD = c++0x
OPTIONS = -std=$(STANDARD) -pthread -Wall
OBJECTS = TestSuite.o MultiTest.o

compile: $(OBJECTS)

TestSuite.o: TestSuite.h
	$(CC) $(OPTIONS) -c TestSuite.h

MultiTest.o: MultiTest.h
	$(CC) $(OPTIONS) -c MultiTest.h

install: TestSuite.o MultiTest.o
	cp TestSuite.h /usr/lib
	cp MultiTest.h /usr/lib

install_local: TestSuite.o MultiTest.o
	cp TestSuite.h /usr/local/lib
	cp MultiTest.h /usr/local/lib

clean:
	rm -f $(OBJECTS) *~ *.swp

