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

/**
 * @brief Check if two strings match
 * @param a first string
 * @param b second string
 * @return whether the two strings match or not
*/
bool
yadsl_testerutils_match(
	const char* a,
	const char* b);

/**
 * @brief Check if two strings don't match
 * @param a first string
 * @param b second string
 * @return whether the two strings don't match
*/
bool
yadsl_testerutils_unmatch(
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
 * @param string string containing YES or NO
 * @return boolean value. If neigher YES or NO are present, NO is assumed.
*/
bool
yadsl_testerutils_str_to_bool(
	const char* string);

/** @} */

#endif
