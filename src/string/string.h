#ifndef __YADSL_STRING_H__
#define __YADSL_STRING_H__

/**
 * \defgroup string String
 * @brief String manipulation
*/

/**
 * @brief Duplicate string
 * @param string base string
 * @return duplicated string or NULL on failure
*/
char*
yadsl_string_duplicate(
		const char* string);

/**
 * @brief Compare C strings ignoring case
 * @param string_a first string
 * @param string_b second string
 * @return analogous to strcmp
*/
int
yadsl_string_compare_ic(
	const char* string_a,
	const char* string_b);

#endif