#ifndef __YADSL_UTILS_DIFF_H__
#define __YADSL_UTILS_DIFF_H__

/**
* \defgroup utils Utilities
* @brief Utility functions
* @{
*/

#include <stddef.h>

/**
 * @brief Scores the difference between two strings
 * taking into account the proximity of the
 * characters on the keyboard that were mistaken
 * @param a first string (not NULL)
 * @param alen length of a
 * @param b second string (not NULL)
 * @param blen length of b
 * @return the difference between the strings or
 * a negative number if could not allocate enough memory
*/
double
yadsl_utils_diff(
    const char* a,
    size_t alen,
    const char* b,
    size_t blen);

/** @} */

#endif
