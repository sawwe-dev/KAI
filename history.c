#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kai.h"
#include "history.h"

int load_from_file(char *filename, History *hist){
    FILE *f;
    char buf[2056];
    int size;
    char *start, *end, *usr = NULL, *msg = NULL;

    printf("Loading chat from file %s\n", filename);

    f = fopen(filename, "r");
    if(!f){
        perror("Error opening chat file");
        return EXIT_FAILURE;
    }
    while(fgets(buf, sizeof(buf), f)){
        /* Si la linea empieza con [ es un nuevo mensaje */
        if(buf[0] == '[' && strstr(buf, "]: ")){
            if(usr && msg){
                /* Guardamos el anterior */
                add_message(hist, usr, msg);
                free(usr);
                free(msg);
            }
            /* Guardamos usuario */
            start = strchr(buf, '[');
            end = strchr(buf, ']');
            size = end - start -1;
            usr = malloc(sizeof(char) * (size + 1));
            strncpy(usr, start + 1, size);
            usr[size] = '\0';
            /* Guardamos inicio del mensaje */
            start = end + 3; /* Primer char despues de ]: */
            end = buf + strlen(buf) - 1; /* caracter anterior al \n */
            size = end - start;
            msg = malloc(sizeof(char) * (size + 1));
            strncpy(msg, start, size + 1);
            msg[size] = '\0';
        }else{
            buf[strlen(buf)-1] = '\0'; /* Para no copiar el último newline */
            size = strlen(msg) + strlen(buf) + 2;
            msg = realloc(msg, size);
            strcat(msg, "\n");
            strcat(msg, buf);
            msg[size-1] = '\0';
        }
    }

    add_message(hist, usr, msg);
    free(usr);
    free(msg);
    fclose(f);

    return EXIT_SUCCESS;
}

void dump_history(History *h){
    FILE *f;
    int i;
    char filename[MAX_USR_INPUT], aux[MAX_FILENAME], msg[MAX_USR_INPUT + 20];
    
    print_system("Insert name of the file to save the chat: ");
    fgets(aux, MAX_FILENAME, stdin);

    i = 0;
    while(aux[i] != '\n'){
        i++;
    }
    aux[i] = '\0';

    sprintf(filename, "chats/%s", aux);

    f = fopen(filename, "w");
    if(!f){
        perror("Error opening file");
        return;
    }

    for(i = 0; i<h->count; i++){
        fprintf(f, "[%s]: %s\n", h->msgs[i].role, h->msgs[i].content);
    }

    sprintf(msg, "Chat saved in %s\n", filename);

    print_system(msg);

    fclose(f);
}

void add_message(History *hist, char *role, char *content){
    int aux_size;
    if(hist->count >= hist->capacity){
        hist->capacity *= 2;
        hist->msgs = realloc(hist->msgs, hist->capacity * sizeof(Message));
        if(!hist->msgs){
            perror("Error reallocating chat history");
            return;
        }
    }

    /* No va con strdup */
    aux_size = strlen(role);
    hist->msgs[hist->count].role = malloc((aux_size + 1) * sizeof(char));
    hist->msgs[hist->count].role[aux_size] = '\0';
    strcpy(hist->msgs[hist->count].role, role);
    aux_size = strlen(content);
    hist->msgs[hist->count].content = malloc((aux_size + 1) * sizeof(char));
    hist->msgs[hist->count].content[aux_size] = '\0';
    strcpy(hist->msgs[hist->count].content, content);
    hist->count++;
}

void free_history(History *hist){
    int i;
    for(i = 0; i<hist->count; i++){
        free(hist->msgs[i].role);
        free(hist->msgs[i].content);
    }
    free(hist->msgs);
    free(hist);
}

void load_chat(History *h, char **chat_buffer){
    int i, size = 1024, needed, start;
    char *chat, *role, *content;

    chat = malloc(size * sizeof(char));
    if(!chat){
        return;
    }
    chat[0] = '\0';

    start = (h->count > MAX_MSGS)? h->count - MAX_MSGS: 0;

    for(i = start; i<h->count; i++){
        role = h->msgs[i].role;
        content = h->msgs[i].content;
        needed = strlen(chat) + strlen(role) + strlen(content) + 4;
        while(needed>size){
            size *= 2;
            chat = realloc(chat, size);
        }
        strcat(chat, " [");
        strcat(chat, role);
        strcat(chat, "]: ");
        strcat(chat, content);
    }

#ifdef DEBUG
    printf("----Chat %d----\n%s\n------------\n", h->count, chat);
#endif

    clean_string(chat);
    
    if(*chat_buffer != NULL) free(*chat_buffer);
    *chat_buffer = malloc((strlen(chat) +1 ) * sizeof(char));
    strcpy(*chat_buffer, chat);

    free(chat);
}

void clean_string(char *str){
    while(*str != '\0'){
        if(*str == '\n' || *str == '"') *str = ' ';
        str++;
    }
}
