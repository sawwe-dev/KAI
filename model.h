#ifndef MODEL_H
#define MODEL_H

#include <stdio.h>
#include <curl/curl.h>

#define API_KEY "HF_API_KEY"
#define AI_MODEL "llama3-8b-8192"
#define API_URL "https://router.huggingface.co/groq/openai/v1/chat/completions"

#define AUTH_HEADER_SIZE 256
#define INIT_PROMPT 256
#define MAX_JSON 4096
#define MAX_SUM 2048

#define PROMPT_INTRO "You are an AI assistant named KAI. For context, here is your conversation with the user: "

typedef struct{
    char *str;
    unsigned int cap;
} GrowStr;

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp);

void build_prompt(GrowStr *prompt, char *user_input, char *chat_summary);

CURL *init_model(char *json_request, char *json_response, struct curl_slist **headers);

CURLcode call_model(CURL *curl, GrowStr *json_request, char *prompt);

int parse_response(char *json_response, char *parsed_response);

#endif
