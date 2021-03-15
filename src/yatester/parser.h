#ifndef yatester_parser_h
#define yatester_parser_h

/* Script Parser */

#include <yatester/err.h>

#include <stdio.h>

yatester_status yatester_initializeparser();
yatester_status yatester_parsescript(FILE *fp);
void yatester_terminateparser();

#endif
