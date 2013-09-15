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

clean:
	rm -f $(OBJECTS) *~ *.swp

