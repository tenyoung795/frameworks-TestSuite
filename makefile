#!/usr/bin/env make -f

CC = g++
LANG = c++-header
STANDARD = c++0x
OPTIONS = -x $(LANG) -std=$(STANDARD) -Wall
LIBRARIES = -pthread
HEADERS = TestSuite.h.gch MultiTest.h.gch

compile: $(HEADERS)

TestSuite.h.gch: TestSuite.h
	$(CC) $(OPTIONS) TestSuite.h $(LIBRARIES)

MultiTest.h.gch: MultiTest.h
	$(CC) $(OPTIONS) MultiTest.h $(LIBRARIES)

install_local: $(HEADERS)
	cp TestSuite.h /usr/local/include
	cp MultiTest.h /usr/local/include

install: $(HEADERS)
	cp TestSuite.h /usr/include
	cp MultiTest.h /usr/include

clean:
	rm -f *~ $(HEADERS) *.swp

