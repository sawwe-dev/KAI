CC = gcc
CFLAGS = -Wall -Wextra -ansi -pedantic -std=c99
CLIBS = -lcurl

EXE = kai

OBJS = kai.o cJSON.o

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(CLIBS)

kai.o: kai.c kai.h
	$(CC) $(CFLAGS) -c $< -o $@

cJSON.o: cJSON.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(EXE) *.o
