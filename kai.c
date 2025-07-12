#include <stdio.h>
#include <curl/curl.h>
#include "cJSON.h"
#include "kai.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
/** TODO: 
 * Meter memoria (chat entero?, si es muy largo resumir)
 */

void clean_string(char *str);

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp){
    size_t total = size * nmemb;
    snprintf((char *)userp, total, "%s", (char*)contents);
    return total;
}

void get_user_input(char *user_input);

void build_prompt(char *prompt, char *user_input);

CURL *init_model(char *json_request, char *json_response, struct curl_slist *headers);

CURLcode call_model(CURL *curl, char *json_request, char *prompt);

void parse_response(char *json_response, char *parsed_response);

void print_response(char *response);

void add_to_history(char *user, char *response, FILE *f);

int main(){
    char user_input[MAX_USR_INPUT] = "";
    char prompt[MAX_PROMPT];
    char json_response[MAX_JSON];
    char json_request[MAX_JSON];
    char parsed_response[MAX_JSON];

    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;

    FILE *f;

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
        build_prompt(prompt, user_input);
        res = call_model(curl, json_request, prompt);
        if(res != CURLE_OK){
            fprintf(stderr, "Curl petition failed: %s\n", curl_easy_strerror(res));
            return EXIT_FAILURE;
        }
        parse_response(json_response, parsed_response);
        print_response(parsed_response);
        add_to_history(user_input, parsed_response, f);
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    curl_global_cleanup();
    fclose(f);
    return EXIT_SUCCESS;
}

void add_to_history(char *user, char *response, FILE *f){
    fprintf(f, "[USER] %s\n[KAI] %s\n", user, response);
}

void print_response(char *response){
    printf("%s> %s %s\n", GREEN, response, RESET);
}

void parse_response(char *json_response, char *parsed_response){
    cJSON *root, *first_choice, *message, *content, *choices;
    root = cJSON_Parse(json_response);
    if (!root) {
        printf("JSON parse error\n");
        return;
    }

    choices = cJSON_GetObjectItem(root, "choices");
    if (!cJSON_IsArray(choices)) {
        printf("Invalid JSON: 'choices' is not an array\n");
        cJSON_Delete(root);
        return;
    }

    first_choice = cJSON_GetArrayItem(choices, 0);
    message = cJSON_GetObjectItem(first_choice, "message");
    content = cJSON_GetObjectItem(message, "content");

    if (cJSON_IsString(content)) {
        strcpy(parsed_response, content->valuestring);
    }

    cJSON_Delete(root);
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

void build_prompt(char *prompt, char *user_input){
    sprintf(prompt, "%s%s", PROMPT_INTRO, user_input);
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
        fprintf(stderr, "Env variable %s not found", API_KEY);
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
