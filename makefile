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

install: compile
	cp TestSuite.h /usr/include/
	cp MultiTest.h /usr/include/

install_local: compile
	cp TestSuite.h /usr/local/include/
	cp MultiTest.h /usr/local/include/

clean:
	rm -f $(OBJECTS) *~ *.swp

