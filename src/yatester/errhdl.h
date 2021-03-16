#ifndef yatester_errhdl_h
#define yatester_errhdl_h

/* Error Handler */

#include <yatester/err.h>

void yatester_pushexpectedstatus(yatester_status status);
yatester_status yatester_getexpectedstatus();
yatester_status yatester_evaluatestatus(yatester_status status);
void yatester_popexpectedstatus();

#endif
