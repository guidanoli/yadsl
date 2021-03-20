#ifndef yatester_parser_h
#define yatester_parser_h

/* Script Parser */

#include <yatester/status.h>

#include <stdio.h>

/**
 * @brief Initialize parser
 * @note Before using the parser, you must initialize it
 * @return operation status
 * @seealso yatester_terminateparser
 */
yatester_status yatester_initializeparser();

/**
 * @brief Check if command name is valid
 * @param commandname command name
 * @return operation status
 */
yatester_status yatester_iscommandnamevalid(const char* commandname);

/**
 * @brief Parse script
 * @param fp script file pointer
 * @return operation status
 */
yatester_status yatester_parsescript(FILE *fp);

/**
 * @brief Terminate parser
 * @note After using the parser, you must terminate it
 * @seealso yatester_initializeparser
 */
void yatester_terminateparser();

#endif
