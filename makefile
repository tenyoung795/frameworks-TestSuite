#!/usr/bin/env make -f

CC = g++
STANDARD = c++0x
OPTIONS = -std=$(STANDARD) -pthread -Wall

compile: TestSuite.cpp TestSuite.h
	$(CC) $(OPTIONS) -c TestSuite.cpp

install: compile
	cp TestSuite.o /usr/lib/
	cp TestSuite.h /usr/include/

install_local: compile
	cp TestSuite.o /usr/local/lib/
	cp TestSuite.h /usr/local/include/

clean:
	rm TestSuite.o *~ *.swp

