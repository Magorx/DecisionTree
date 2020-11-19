CUR_PROG = general

ifndef VERBOSE
.SILENT:
endif

CUR_PROG = detree

GENERAL_PREFIX = general
GC = $(GENERAL_PREFIX)/general_c
GCPP = $(GENERAL_PREFIX)/general_cpp

CC = gcc
CPP = g++

WARNINGS = -Wall -Wextra -Wno-multichar
STANDARD =  
CFLAGS = $(STANDARD) $(WARNINGS) -lm

all: detree

detree: main.cpp gc_strings_and_files.o decision_tree.h $(GCPP)/string.h
	$(CPP) $(CFLAGS) main.cpp gc_strings_and_files.o -g -o detree

gc_strings_and_files.o: $(GC)/strings_and_files.c $(GC)/strings_and_files.h
	$(CPP) $(CFLAGS) $(GC)/strings_and_files.c -c -o gc_strings_and_files.o

run: all
	./$(CUR_PROG)

clear:
	rm *.o