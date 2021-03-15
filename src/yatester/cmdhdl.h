#ifndef yatester_cmdhdl_h
#define yatester_cmdhdl_h

/* Command Handler */

#include <yatester/cmd.h>
#include <yatester/err.h>

yatester_status yatester_initializecmdhdl();
void yatester_itercommands(void (*callback)(const yatester_command*));
const yatester_command* yatester_getcommand(const char* commandname);
void yatester_terminatecmdhdl();

#endif
