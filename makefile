CC=gcc
CFLAGS = -g -Wall -ansi -pedantic

SOURCES =  MainFile.c MacroEliminate.c TwoRuns.c ConvertTo32.c ErrorChecks.c SupportFunctions.c header.h
OBJECTS = $(SOURCES:.c=.o)
TARGET = maman14

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^





