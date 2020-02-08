#ifndef _DIFF_H_
#define _DIFF_H_

#define DIFFERR (((unsigned long) 0) - 1)

/*
* Scores the difference between two strings
* taking into account the proximity of the
* characters on the keyboard that were mistaken
* Returns DIFFERR in case it could not allocate memory
*/
unsigned long diff(const char *s1, const char *s2);

#endif
