#ifndef KAI_H
#define KAI_H

/**
 * @brief Simple boolean structure
 */
typedef enum _bool{NO, YES} Bool;

#define MAX_USR_INPUT 1024 /**< Limit of chars in user input **/
#define MAX_MSGS 30 /**< Limit of messages to prompt **/
#define MAX_FILENAME 64 /**< Limit of name of files **/
#define SHORT_ANSW 16 /**< Limit of user input when asked yes/no questions */

/**
 * @breif Prints system messages. In blue with "[SYSTEM]" tag
 *
 * @param system_msg Pointer to the message
 */
void print_system(char *system_msg);

/* COLORS: */
#define RESET "\033[0m" /**< Ascii reset colors **/
#define GREEN "\033[32m" /**< Ascii green **/
#define RED "\033[91m" /**< Ascii red **/
#define BLUE "\033[0;34m" /**< Ascii blue**/

#define INIT_HIST_CAP 8 /**< Initial history capacity **/

#endif
