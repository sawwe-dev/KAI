#ifndef MODEL_H
#define MODEL_H

#include <stdio.h>
#include <curl/curl.h>

#define API_KEY "HF_API_KEY" /**< Name of the env variable that stores the HF API key **/
#define AI_MODEL "llama3-8b-8192" /**< Ai model used **/
#define API_URL "https://router.huggingface.co/groq/openai/v1/chat/completions" /**< URL to make the requests **/

#define AUTH_HEADER_SIZE 256 /**< Authentication http header size **/
#define INIT_PROMPT 256 /**< Initial prompt size */
#define MAX_JSON 4096 /**< Maximum JSON response limit */

#define PROMPT_INTRO "You are an AI assistant named KAI. For context, here is your conversation with the user: " /**< Prompt introduction for AI request. Used in every call **/

/**
 * @brief Represents a self growing string
 */
typedef struct{
    char *str; /**< Pointer to the string**/
    unsigned int cap; /**< Actual capacity of the string **/
} GrowStr;

/**
 * @brief Callback function used by libcurl to write http response
 *
 * @param contents String containing the response
 * @param size Size of a single data element
 * @param nmemb Number of elements
 * @param userp Pointer to store data
 * @return Size of written data 
 */
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp);

/**
 * @brief Builds prompt from user input and chat history
 *
 * @param prompt Pointer to prompt GrowStr
 * @param user_input Last user input
 * @param chat_summary Last MAX_MSGS in string format
 */
void build_prompt(GrowStr *prompt, char *user_input, char *chat_summary);

/**
 * @brief initializates curl headers and options
 *
 * @param json_request Pointer to the string where the request would be hold
 * @param json_response Pointer to the string where the response would be hold
 * @param headers Pointer to curl_slist headers
 * @return pointer to CURL object
 */
CURL *init_model(char *json_request, char *json_response, struct curl_slist **headers);

/**
 * @brief Makes http petition
 *
 * @param curl CURL object
 * @param json_request Pointer to where the json_request will be stored in memory
 * @param prompt Prompt including user last petition and chat history
 * @return status of the petition
 */
CURLcode call_model(CURL *curl, GrowStr *json_request, char *prompt);

/**
 * @brief Extracts model answer from json response
 *
 * @param json_response Pointer to the http response in JSON format
 * @param parsed_response Pointer to where to write the answer
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int parse_response(char *json_response, char *parsed_response);

#endif
