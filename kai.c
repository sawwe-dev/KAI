#include <curl/curl.h>
#include "cJSON.h"
#include "kai.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/** TODO: 
 * Quitar lo de resumir. String de history que tenga todo el chat?
 * Meter memoria (chat entero?, si es muy largo resumir)
 * Load prompts from a config file
 */

typedef struct{
    char *role;
    char *content;
} Message;

typedef struct{
    Message *msgs;
    int count;
    int capacity;
}History;

void clean_string(char *str);

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp){ size_t total = size * nmemb;
    snprintf((char *)userp, total, "%s", (char*)contents);
    return total;
}

void get_user_input(char *user_input);

void build_prompt(char *prompt, char *user_input, char *chat_summary);

CURL *init_model(char *json_request, char *json_response, struct curl_slist *headers);

CURLcode call_model(CURL *curl, char *json_request, char *prompt);

int parse_response(char *json_response, char *parsed_response);

void print_response(char *response);

void add_to_history(char *user, char *response, FILE *f);

/**
 * Loads in chat_buffer MAX_MSGS messages from the history
 */
void load_chat(History *h, char **chat_buffer);

void print_error(char *error_msg){
    printf("%sREQUEST ERROR: %s%s\n", RED, error_msg, RESET);
}

void free_history(History *hist){
    int i;
    for(i = 0; i<hist->count; i++){
        free(hist->msgs[i].role);
        free(hist->msgs[i].content);
    }
    free(hist->msgs);
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

void dump_history(History *h, FILE *f){
    int i;
    for(i = 0; i<h->count; i++){
        fprintf(f, "[%s]: %s", h->msgs[i].role, h->msgs[i].content);
    }
}

int main(){
    char user_input[MAX_USR_INPUT] = "";
    char prompt[MAX_PROMPT];
    char json_response[MAX_JSON];
    char json_request[MAX_JSON];
    char parsed_response[MAX_JSON];
    char *chat_buffer = NULL;

    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;

    FILE *f;

    History *hist;

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

    curl = init_model(json_request, json_response, headers);
    if(!curl){
        return EXIT_FAILURE;
    }

    f = fopen(HIST_FILENAME, "r+");
    if(!f){
        perror("Error opening history file");
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        curl_global_cleanup();
        return EXIT_FAILURE;
    }

    while(strcmp(user_input, "exit")){
        get_user_input(user_input);
        add_message(hist, "USER", user_input);
        build_prompt(prompt, user_input, chat_buffer);
        res = call_model(curl, json_request, prompt);
        if(res != CURLE_OK){
            fprintf(stderr, "Curl petition failed: %s\n", curl_easy_strerror(res));
            return EXIT_FAILURE;
        }
        printf("\nJson response: %s\n\n", json_response);
        if(parse_response(json_response, parsed_response) == EXIT_FAILURE){
            print_error(parsed_response);
            break;
        }
        print_response(parsed_response);
        add_message(hist, "KAI", parsed_response);
        load_chat(hist, &chat_buffer);
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    curl_global_cleanup();
    fclose(f);
    free_history(hist);
    return EXIT_SUCCESS;
}

void add_to_history(char *user, char *response, FILE *f){
    fprintf(f, "[USER] %s\n[KAI] %s\n", user, response);
}

void print_response(char *response){
    printf("%s> %s %s\n", GREEN, response, RESET);
}

int parse_response(char *json_response, char *parsed_response){
    cJSON *root, *first_choice, *message, *content, *choices;
    root = cJSON_Parse(json_response);
    if (!root) {
        printf("JSON parse error\n");
        return EXIT_FAILURE;
    }

    choices = cJSON_GetObjectItem(root, "choices");
    if (!cJSON_IsArray(choices)) {
        choices = cJSON_GetObjectItem(root, "error");
        message = cJSON_GetObjectItem(choices, "message");
        if(cJSON_IsString(message)) 
            strcpy(parsed_response, message->valuestring);
        cJSON_Delete(root);
        return EXIT_FAILURE;
    }

    first_choice = cJSON_GetArrayItem(choices, 0);
    message = cJSON_GetObjectItem(first_choice, "message");
    content = cJSON_GetObjectItem(message, "content");

    if (cJSON_IsString(content)) {
        strcpy(parsed_response, content->valuestring);
    }

    cJSON_Delete(root);
    return EXIT_SUCCESS;
}

CURLcode call_model(CURL *curl, char *json_request, char *prompt){
    CURLcode res;
    sprintf(json_request, "{\"model\": \"%s\", \"messages\": [{\"role\": \"user\", \"content\": \"%s\"}]}", AI_MODEL, prompt);
    res = curl_easy_perform(curl);
    return res;
}

void get_user_input(char *user_input){
    printf("> ");
    fgets(user_input, MAX_USR_INPUT, stdin);
    while(*user_input != '\n'){
        user_input++;
    }
    *user_input = '\0';
}

void build_prompt(char *prompt, char *user_input, char *chat_summary){
    if(chat_summary != NULL){
        sprintf(prompt, "You are an AI named KAI. Here is your chat with the user: %s [USER]: %s",chat_summary, user_input);
    }else{
        /* First interaction */
        sprintf(prompt, "%s %s", PROMPT_INTRO, user_input);
    }
    printf("\n----->Prompted: %s\n\n", prompt);
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
        needed = strlen(chat) + strlen(role) + strlen(content);
        while(needed>size){
            size *= 2;
            chat = realloc(chat, size);
        }
        strcat(chat, " [");
        strcat(chat, role);
        strcat(chat, "]: ");
        strcat(chat, content);
    }

    printf("----Chat %d----\n%s\n------------\n", h->count, chat);

    clean_string(chat);
    
    if(*chat_buffer != NULL) free(*chat_buffer);
    *chat_buffer = malloc((strlen(chat) +1 ) * sizeof(char));
    strcpy(*chat_buffer, chat);
}

void clean_string(char *str){
    while(*str != '\0'){
        if(*str == '\n') *str = ' ';
        str++;
    }
}

CURL *init_model(char *json_request, char *json_response, struct curl_slist *headers){
    char *api_token;
    CURL *curl;
    char auth_header[AUTH_HEADER_SIZE];

    api_token = getenv(API_KEY);
    if(!api_token){
        fprintf(stderr, "Env variable %s not found\n", API_KEY);
        return NULL;
    }
    sprintf(auth_header, "Authorization: Bearer %s", api_token);

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(!curl){
        fprintf(stderr, "Curl failed to init.");
        return NULL;
    }

    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth_header);

    curl_easy_setopt(curl, CURLOPT_URL, API_URL);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_request);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, json_response);

    return curl;
}
