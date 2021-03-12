#include <yatester/errhdl.h>

#include <setjmp.h>

static jmp_buf env;

yatester_status yatester_catch()
{
	return setjmp(env);
}

void yatester_throw(yatester_status status)
{
	longjmp(env, status);
}
