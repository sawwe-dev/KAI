CC = gcc
CFLAGS = -Wall -Wextra -ansi -pedantic -std=c99
CLIBS = -lcurl

EXE = kai

OBJS = kai.o cJSON.o model.o history.o

all: $(EXE)

debug: CFLAGS += -DDEBUG
debug: clean $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(CLIBS)

kai.o: kai.c kai.h
	$(CC) $(CFLAGS) -c $< -o $@

model.o: model.c model.h
	$(CC) $(CFLAGS) -c $< -o $@

history.o: history.c history.h
	$(CC) $(CFLAGS) -c $< -o $@

cJSON.o: cJSON.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(EXE) *.o
