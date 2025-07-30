#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "model.h"

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp){ 
    size_t total = size * nmemb;
    snprintf((char *)userp, total, "%s", (char*)contents);
    return total;
}

void build_prompt(char *prompt, char *user_input, char *chat_summary){
    if(chat_summary != NULL){
        sprintf(prompt, "%s %s. And here is the last user message: %s. Answer",PROMPT_INTRO, chat_summary, user_input);
    }else if(!strcmp(user_input, "exit")){
        printf("EXITING\n");
        sprintf(prompt, "%s %s. User says bye. Say bye to the user.", PROMPT_INTRO, chat_summary);
    } else{
        /* First interaction */
        sprintf(prompt, "%s %s", PROMPT_INTRO, user_input);
    }
#ifdef DEBUG
    printf("\n----->Prompted: %s\n\n", prompt);
#endif
    if(strlen(prompt) > MAX_PROMPT){
        printf("ERROR, size exceeded.\n");
        exit(0);
    }
}

CURL *init_model(char *json_request, char *json_response, struct curl_slist **headers){
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

    *headers = curl_slist_append(*headers, "Content-Type: application/json");
    *headers = curl_slist_append(*headers, auth_header);

    curl_easy_setopt(curl, CURLOPT_URL, API_URL);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, *headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_request);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, json_response);

    return curl;
}

CURLcode call_model(CURL *curl, char *json_request, char *prompt){
    CURLcode res;
    sprintf(json_request, "{\"model\": \"%s\", \"messages\": [{\"role\": \"user\", \"content\": \"%s\"}]}", AI_MODEL, prompt);
    res = curl_easy_perform(curl);
    return res;
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
