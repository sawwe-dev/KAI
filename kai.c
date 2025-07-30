#define _POSIX_C_SOURCE 200909L /* For getopt to work */
#include <curl/curl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "kai.h"
#include "history.h"
#include "model.h"

Bool ask_usr(char *str);

void get_user_input(char *user_input);

int parse_response(char *json_response, char *parsed_response);

void print_response(char *response);

void parse_code(char *parsed_response);

void print_error(char *error_msg);

int main(int argc, char *argv[]){
    char user_input[MAX_USR_INPUT] = "";
    char *prompt;
    char json_response[MAX_JSON];
    char json_request[MAX_JSON];
    char parsed_response[MAX_JSON];
    char *chat_buffer = NULL;

    int opt;

    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;

    History *hist;

    prompt = malloc(sizeof(char) * INIT_PROMPT);
    if(!prompt){
        return EXIT_FAILURE;
    }

    hist = malloc(sizeof(History));
    if(!hist){
        perror("Error allocating memory for chat history.");
        return EXIT_FAILURE;
    }
    
    hist->count = 0;
    hist->capacity = INIT_HIST_CAP;
    hist->msgs = malloc(hist->capacity * sizeof(Message));
    if(!hist->msgs){
        perror("Error allocating memory for chat history.");
        return EXIT_FAILURE;
    }

    curl = init_model(json_request, json_response, &headers);
    if(!curl){
        return EXIT_FAILURE;
    }

    if(argc > 0){
        while((opt = getopt(argc, argv, "l:")) != -1){
            switch(opt){
                case 'l':
                    /* Load chat from file */
                    load_from_file(optarg, hist);
                    load_chat(hist, &chat_buffer);
                    break;
                case '?':
                    /* Undetermined */
                    break;
            }
        }
    }

    while(strcmp(user_input, "exit")){
        get_user_input(user_input);
        add_message(hist, "USER", user_input);
        build_prompt(prompt, user_input, chat_buffer);
        printf("Prompt size: %zu\n\n", strlen(prompt));
        res = call_model(curl, json_request, prompt);
        if(res != CURLE_OK){
            fprintf(stderr, "Curl petition failed: %s\n", curl_easy_strerror(res));
            return EXIT_FAILURE;
        }
#ifdef DEBUG
        printf("\nJson response: %s\n\n", json_response);
#endif
        if(parse_response(json_response, parsed_response) == EXIT_FAILURE){
            print_error(parsed_response);
            printf("Json request: %s\nJson response: %s\n", json_request, json_response);
            break;
        }
        print_response(parsed_response);
        parse_code(parsed_response);
        add_message(hist, "KAI", parsed_response);
        load_chat(hist, &chat_buffer);
    }

    if(ask_usr("Chat finished, save chat?") == YES){
        dump_history(hist);
    }

    if(chat_buffer) free(chat_buffer);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    curl_global_cleanup();
    free_history(hist);
    free(prompt);
    return EXIT_SUCCESS;
}

void print_response(char *response){
    printf("%s> %s %s\n", GREEN, response, RESET);
}

void get_user_input(char *user_input){
    printf("> ");
    fgets(user_input, MAX_USR_INPUT, stdin);
    while(*user_input != '\n'){
        user_input++;
    }
    *user_input = '\0';
}

void parse_code(char *parsed_response){
    int len, i, start, end;
    char filename[MAX_USR_INPUT], aux[MAX_FILENAME], msg[MAX_USR_INPUT+20];
    FILE *f;

    len = strlen(parsed_response);
    /* -6 cos code cant be the last 6 chars */
    for(i = 0, start = -1; i<len-6; i++){
        if(parsed_response[i] == '`' && parsed_response[i+1] == '`' && parsed_response[i+2] == '`'){
            /* md code found */
            start = i;
            break;
        }
    }

    if(start == -1){
        /* No code found */
        return;
    }

    for(i = start+3, end = -1; i<len; i++){
        if(parsed_response[i] == '`' && parsed_response[i+1] == '`' && parsed_response[i+2] == '`'){
            /* Code end */
            end = i;
            break;
        }
    }

    if(end == -1){
        /* No code end (impossible) */
        printf("No code end found \n");
        return;
    }

    i = start + 3;
    while(parsed_response[i] != '\n'){
        i++;
    }
    start = i+1; /* First char of code */

    if(ask_usr("Code founded in response. Extract code to file?") == NO){
        return;
    }

    /* Extract code to file */
    print_system("Insert name of the file to extract code: ");
    fgets(aux, MAX_FILENAME, stdin);

    i = 0;
    while(aux[i] != '\n'){
        i++;
    }
    aux[i] = '\0';

    sprintf(filename, "code/%s", aux);

    sprintf(msg, "Writing code into %s\n", filename);

    print_system(msg);

    f = fopen(filename, "w");
    if(!f){
        perror("Error opening file");
        return;
    }

    for(i = start; i<end; i++){
        fputc(parsed_response[i], f);
    }

    fclose(f);
}

void print_error(char *error_msg){
    printf("%sREQUEST ERROR: %s%s\n", RED, error_msg, RESET);
}

void print_system(char *system_msg){
    printf("%s[SYSTEM] %s%s", BLUE, system_msg, RESET);
}

Bool ask_usr(char *str){
    char usr[SHORT_ANSW];
    printf("%s[SYSTEM] %s [y/n]: %s",BLUE, str, RESET);
    fgets(usr, SHORT_ANSW, stdin);
    if(usr[0] == 'Y' || usr[0] == 'y') return YES;
    return NO;
}
