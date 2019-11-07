#ifndef _DIFF_H_
#define _DIFF_H_

/*
* Scores the difference between two strings
* taking into account the proximity of the
* characters on the keyboard that were mistaken
* Returns -1 in case it could not allocate memory
*/
int diff(char *s1, char *s2);

#endif
