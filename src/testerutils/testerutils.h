#ifndef __YADSL_TESTERUTILS_H__
#define __YADSL_TESTERUTILS_H__

/**
 * \defgroup testerutils Tester Utilities
 * @brief Auxiliary module for Tester
 *
 * @{
*/

#include <stdbool.h>
#include <stdio.h>

#include <tester/tester.h>

typedef void (*yadsl_TesterUtilsCommandHandler)();

typedef struct
{
    const char* command;
    void (*commandhandler)();
}
yadsl_TesterUtilsCommand;

/**
 * @brief Check if two strings match, ignoring case
 * @param a first string
 * @param b second string
 * @return whether the two strings match or not
*/
bool
yadsl_testerutils_match(
	const char* a,
	const char* b);

/**
 * @brief Serializes string to file pointer
 * @param file file pointer opened in writing mode
 * @param string string to be serialized
 * @return true on failure
*/
bool
yadsl_testerutils_str_serialize(
	FILE* file,
	const char* string);

/**
 * @brief Deserializes string from file pointer
 * @param file file pointer opened in reading mode
 * @return serialized string or NULL on failure
*/
char*
yadsl_testerutils_str_deserialize(
	FILE* file);

/**
 * @brief Convert string to bool
 * @param string boolean value in words (case insensitive)
 * @return boolean value (NO is the default)
*/
bool
yadsl_testerutils_str_to_bool(
	const char* string);

/**
 * @brief Add temporary file to list
 * [!] On failure, tries to remove file. Be sure to close any remaining
 *     file pointers before calling this function
 * @param filename temporary file name
 * @return sucess (true) or failure (false)
*/
bool
yadsl_testerutils_add_tempfile_to_list(
	const char* filename);

/**
 * @brief Removes all temporary files in the list
*/
void
yadsl_testerutils_clear_tempfile_list();

/**
 * @brief Compares file and string
 * @param fp file pointer
 * @param string string
 * @return if file contents and string are equal
*/
bool
yadsl_testerutils_compare_file_and_string(
    FILE* fp,
    const char* string);

/**
 * @brief Parse command
 * Hint: Order the commands by ascending order of
 * length so that parsing time is optimized
 * @param command command
 * @param commands NULL-terminated array of commands
 * @return return code
*/
yadsl_TesterRet
yadsl_testerutils_parse_command(
    const char* command,
    yadsl_TesterUtilsCommand* commands);

/** @} */

#endif
