#ifndef _AA_UTILS_DIFF_H_
#define _AA_UTILS_DIFF_H_

/* Scores the difference between two strings
   taking into account the proximity of the
   characters on the keyboard that were mistaken
   Returns -1.0 in case it could not allocate memory
*/
double aa_utils_diff(const char *s1, const char *s2);

#endif
