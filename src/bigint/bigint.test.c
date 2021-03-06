#include <bigint/bigint.h>

#include <tester/tester.h>
#include <testerutils/testerutils.h>

#include <stddef.h>

const char *yadsl_tester_help_strings[] = {0};

#define MAXSTACKSIZE 16
yadsl_BigIntHandle* stack[MAXSTACKSIZE];
int top;

yadsl_TesterRet yadsl_tester_init()
{
	return YADSL_TESTER_RET_OK;
}

static void checkstack()
{
	yadsl_tester_assertx(top < MAXSTACKSIZE, "stack overflow");
}

static void push(yadsl_BigIntHandle* bigint)
{
	checkstack();
	stack[top++] = bigint;
}

static yadsl_BigIntHandle* pop()
{
	yadsl_tester_assertx(top > 0, "empty stack");
	return stack[--top];
}

static void checkindex(int index)
{
	yadsl_tester_assertx(index >= 0, "index < 0");
	yadsl_tester_assertx(index < top, "index >= top");
}

static yadsl_BigIntHandle* at(int index)
{
	checkindex(index);
	return stack[index];
}

static void bigint_push()
{
	intmax_t i;
	yadsl_BigIntHandle* bigint;
	yadsl_tester_parse_n_arguments("I", &i);
	checkstack();
	bigint = yadsl_bigint_from_int(i);
	yadsl_tester_assert(bigint != NULL, YADSL_TESTER_RET_MALLOC);
	push(bigint);
}

static void bigint_pop()
{
	yadsl_bigint_destroy(pop());
}

static void bigint_settop()
{
	int n;
	yadsl_tester_parse_n_arguments("i", &n);
	checkindex(n);
	while (top != n) yadsl_bigint_destroy(pop());
}

static void bigint_gettop()
{
	int n;
	yadsl_tester_parse_n_arguments("i", &n);
	yadsl_tester_asserteqi(n, top, NULL);
}

static void bigint_get()
{
	int n;
	intmax_t expected, obtained;
	yadsl_tester_parse_n_arguments("iI", &n, &expected);
	checkindex(n);
	yadsl_tester_assert(yadsl_bigint_to_int(at(n), &obtained), YADSL_TESTER_RET_OVERFLOW);
	yadsl_tester_asserteqI(expected, obtained, NULL);
}

static void bigint_roundtrip()
{
	intmax_t i, j;
	yadsl_BigIntHandle* bigint;
	yadsl_tester_parse_n_arguments("I", &i);
	checkstack();
	bigint = yadsl_bigint_from_int(i);
	yadsl_tester_assert(bigint != NULL, YADSL_TESTER_RET_MALLOC);
	push(bigint);
	yadsl_tester_assert(yadsl_bigint_to_int(at(top-1), &j), YADSL_TESTER_RET_OVERFLOW);
	yadsl_tester_asserteqI(i, j, NULL);
	bigint_pop();
}

static void bigint_copy()
{
	int n;
	yadsl_BigIntHandle* copy;
	yadsl_tester_parse_n_arguments("i", &n);
	checkindex(n);
	checkstack();
	copy = yadsl_bigint_copy(at(n));
	yadsl_tester_assert(copy != NULL, YADSL_TESTER_RET_MALLOC);
	push(copy);
}

static void bigint_copyroundtrip()
{
	intmax_t i, j;
	yadsl_BigIntHandle* copy;
	bigint_push();
	yadsl_tester_assert(yadsl_bigint_to_int(at(top-1), &i), YADSL_TESTER_RET_OVERFLOW);
	checkstack();
	copy = yadsl_bigint_copy(at(top-1));
	bigint_pop();
	yadsl_tester_assert(copy != NULL, YADSL_TESTER_RET_MALLOC);
	push(copy);
	yadsl_tester_assert(yadsl_bigint_to_int(at(top-1), &j), YADSL_TESTER_RET_OVERFLOW);
	bigint_pop();
	yadsl_tester_asserteqI(i, j, NULL);
}


#define CMD(name) { #name, bigint_ ## name }

static yadsl_TesterUtilsCommand commands[] = {
	CMD(push),
	CMD(pop),
	CMD(settop),
	CMD(gettop),
	CMD(get),
	CMD(roundtrip),
	CMD(copy),
	CMD(copyroundtrip),
	{ NULL, NULL },
};

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
	return yadsl_testerutils_parse_command(command, commands);
}

yadsl_TesterRet yadsl_tester_release()
{
	while (top != 0)
		yadsl_bigint_destroy(pop());

	return YADSL_TESTER_RET_OK;
}
