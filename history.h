#ifndef HISTORY_H
#define HISTORY_H

#include <stdio.h>

/**
 * @brief Represents a message in a conversation
 */
typedef struct{
    char *role; /**< Who **/
    char *content; /**< What **/
} Message;

/**
 * @brief Stores the chat history between the user and the AI
 */
typedef struct{
    Message *msgs; /**< Array of messages **/
    int count; /**< Number of messages in the array **/
    int capacity; /**< Capacity of the array **/
}History;

/**
 * @brief Loads chat from a chat file into a History struct
 *
 * @param filename Name of the file to extract chat history
 * @param hist Pointer to history structure
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int load_from_file(char *filename, History *hist);

/**
 * @brief Adds a message to the history
 *
 * @param hist Pointer to history structure
 * @param role Who send the message
 * @param content Message to add
 */
void add_message(History *hist, char *role, char *content);

/**
 * @brief Loads whole chat history into a string
 *
 * @param h Pointer to history struct
 * @param chat_buffer Pointer to the string
 */
void load_chat(History *h, char **chat_buffer);

/**
 * @brief Frees history struct
 *
 * @param hist Pointer to history struct 
 */
void free_history(History *hist);

/**
 * @brief Asks user a file and dumps history into file
 *
 * @param h Pointer to history struct
 */
void dump_history(History *h);

/**
 * @brief Substitutes newlines and '"' with whitespace in a string
 *
 * @param str String to clean
 */
void clean_string(char *str);

#endif
