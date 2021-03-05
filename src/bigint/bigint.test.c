#include <bigint/bigint.h>

#include <tester/tester.h>
#include <testerutils/testerutils.h>

#include <stddef.h>

const char *yadsl_tester_help_strings[] = {0};

#define MAXSTACKSIZE 16
yadsl_BigIntHandle* stack[MAXSTACKSIZE];
size_t stacksize;

yadsl_TesterRet yadsl_tester_init()
{
	return YADSL_TESTER_RET_OK;
}

/* TODO: push and pop static functions */

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
	if (yadsl_testerutils_match(command, "push")) {
		intmax_t i;
		yadsl_BigIntHandle* bigint;
		if (yadsl_tester_parse_arguments("I", &i) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if (stacksize == MAXSTACKSIZE)
			return yadsl_tester_return_external_value("stack overflow");
		bigint = yadsl_bigint_from_int(i);
		if (bigint == NULL)
			return YADSL_TESTER_RET_MALLOC;
		stack[stacksize++] = bigint;
	} else if (yadsl_testerutils_match(command, "pop")) {
		int n;
		if (yadsl_tester_parse_arguments("i", &n) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if (n < 0)
			return yadsl_tester_return_external_value("negative pop argument");
		if (n >= stacksize)
			return yadsl_tester_return_external_value("empty stack");
		for (int i = stacksize - 1; i >= stacksize - n; --i)
			yadsl_bigint_destroy(stack[i]);
		stacksize -= n;
	} else {
		return YADSL_TESTER_RET_COUNT;
	}
	return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_release()
{
	while (stacksize > 0)
		yadsl_bigint_destroy(stack[--stacksize]);

	return YADSL_TESTER_RET_OK;
}
