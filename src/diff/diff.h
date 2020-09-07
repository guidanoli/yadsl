#ifndef __YADSL_UTILS_DIFF_H__
#define __YADSL_UTILS_DIFF_H__

/**
* \defgroup utils Utilities
* @brief Utility functions
* @{
*/

/**
 * @brief Scores the difference between two strings
 * taking into account the proximity of the
 * characters on the keyboard that were mistaken
 * @param s1 first string
 * @param s2 second string
 * @return the difference between the strings or
 * -1.0 in case it could not allocate enough memory
*/
double
yadsl_utils_diff(
	const char* s1,
	const char* s2);

/** @} */

#endif
