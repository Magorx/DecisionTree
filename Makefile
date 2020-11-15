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

detree: main.cpp general.h decision_tree.o gc_strings_and_files.o
	$(CPP) $(CFLAGS) main.cpp decision_tree.o gc_strings_and_files.o -o detree

decision_tree.o: decision_tree.cpp decision_tree.h $(GCPP)/vector.h $(GCPP)/string.h
	$(CPP) $(CFLAGS) decision_tree.cpp -c -o decision_tree.o

gc_strings_and_files.o: $(GC)/strings_and_files.c $(GC)/strings_and_files.h
	$(CPP) $(CFLAGS) $(GC)/strings_and_files.c -c -o gc_strings_and_files.o

run: all
	./$(CUR_PROG)

clear:
	rm *.o