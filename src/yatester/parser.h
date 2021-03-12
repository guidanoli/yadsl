#ifndef yatester_parser_h
#define yatester_parser_h

/* Script Parser */

#include <yatester/err.h>

#include <stdio.h>

#define MAXARGCNT 64
#define MAXARGLEN BUFSIZ
#define MAXCMDLEN BUFSIZ

yatester_status yatester_parsescript(FILE *fp);

#endif
