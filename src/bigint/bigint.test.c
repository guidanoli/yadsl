#include <bigint/bigint.h>

#include <tester/tester.h>
#include <testerutils/testerutils.h>

#include <stddef.h>

const char *yadsl_tester_help_strings[] = {0};

#define MAXSTACKSIZE 16
yadsl_BigIntHandle* stack[MAXSTACKSIZE];
int stacksize;

static void check_index(int index)
{
	yadsl_tester_assertx(index >= 0, "negative index");
	yadsl_tester_assertx(index < stacksize, "index too big");
}

yadsl_TesterRet yadsl_tester_init()
{
	return YADSL_TESTER_RET_OK;
}

/* TODO: push and pop static functions */

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
	yadsl_TesterRet ret;
	if (yadsl_testerutils_match(command, "push")) {
		intmax_t i;
		yadsl_BigIntHandle* bigint;
		yadsl_tester_parse_n_arguments("I", &i);
		yadsl_tester_assertx(stacksize < MAXSTACKSIZE, "stack overflow");
		bigint = yadsl_bigint_from_int(i);
		yadsl_tester_assert(bigint != NULL, YADSL_TESTER_RET_MALLOC);
		stack[stacksize++] = bigint;
	} else if (yadsl_testerutils_match(command, "pop")) {
		int n;
		yadsl_tester_parse_n_arguments("i", &n);
		yadsl_tester_assertx(n >= 0, "negative pop argument");
		yadsl_tester_assertx(n <= stacksize, "empty stack");
		for (int i = stacksize - 1; i >= stacksize - n; --i)
			yadsl_bigint_destroy(stack[i]);
		stacksize -= n;
	} else if (yadsl_testerutils_match(command, "settop")) {
		int n;
		yadsl_tester_parse_n_arguments("i", &n);
		check_index(n);
		while (stacksize != n)
			yadsl_bigint_destroy(stack[--stacksize]);
	} else if (yadsl_testerutils_match(command, "gettop")) {
		int n;
		yadsl_tester_parse_n_arguments("i", &n);
		yadsl_tester_asserteqi(n, stacksize, NULL);
	} else if (yadsl_testerutils_match(command, "get")) {
		int n;
		intmax_t expected, obtained;
		yadsl_tester_parse_n_arguments("iI", &n, &expected);
		check_index(n);
		yadsl_tester_assert(yadsl_bigint_to_int(stack[n], &obtained), YADSL_TESTER_RET_OVERFLOW);
		yadsl_tester_asserteqI(expected, obtained, NULL);
	} else if (yadsl_testerutils_match(command, "roundtrip")) {
		intmax_t i, j;
		yadsl_BigIntHandle* bigint;
		yadsl_tester_parse_n_arguments("I", &i);
		yadsl_tester_assertx(stacksize < MAXSTACKSIZE, "stack overflow");
		bigint = yadsl_bigint_from_int(i);
		yadsl_tester_assert(bigint != NULL, YADSL_TESTER_RET_MALLOC);
		stack[stacksize++] = bigint;
		yadsl_tester_assert(yadsl_bigint_to_int(stack[stacksize-1], &j), YADSL_TESTER_RET_OVERFLOW);
		yadsl_tester_asserteqI(i, j, NULL);
		yadsl_bigint_destroy(stack[--stacksize]);
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
