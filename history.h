#ifndef HISTORY_H
#define HISTORY_H

#include <stdio.h>

typedef struct{
    char *role;
    char *content;
} Message;

typedef struct{
    Message *msgs;
    int count;
    int capacity;
}History;

int load_from_file(char *filename, History *hist);

void add_to_history(char *user, char *response, FILE *f);

void load_chat(History *h, char **chat_buffer);

void free_history(History *hist);

void add_message(History *hist, char *role, char *content);

void dump_history(History *h);

void clean_string(char *str);

#endif
