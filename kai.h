#ifndef KAI_H
#define KAI_H

#define HIST_FILENAME "chats/"

typedef enum _bool{NO, YES} Bool;

#define MAX_USR_INPUT 1024
#define MAX_MSGS 30
#define MAX_EXT 10
#define MAX_FILENAME 64
#define SHORT_ANSW 16

void print_system(char *system_msg);

/* COLORS: */
#define RESET "\033[0m"
#define GREEN "\033[32m"
#define RED "\033[91m"
#define BLUE "\033[0;34m"

#define INIT_HIST_CAP 8

#endif
