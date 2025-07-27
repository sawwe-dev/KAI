#ifndef KAI_H
#define KAI_H

#define API_KEY "HF_API_KEY"
#define AI_MODEL "llama3-8b-8192"
#define API_URL "https://router.huggingface.co/groq/openai/v1/chat/completions"

#define HIST_FILENAME "chats/example.chat"
#define PROMPTS_FILENAME ".config/prompts.json"

#define MAX_USR_INPUT 1024
#define MAX_PROMPT 2048
#define MAX_JSON 4096
#define MAX_SUM 2048
#define AUTH_HEADER_SIZE 256
#define MAX_MSGS 3

#define PROMPT_INTRO "You are an AI named KAI:"
#define SUM_INSTRUCTIONS "Your task is to summarize this conversation between an AI assistant named KAI and its user. Focus on user's interests and information. Write a short summary all in the same line:"

/* COLORS: */
#define RESET "\033[0m"
#define GREEN "\033[32m"

#define INIT_HIST_CAP 8

#endif
