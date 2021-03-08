#include <bigint/bigint.h>

#include <tester/tester.h>
#include <testerutils/testerutils.h>

#include <stddef.h>

#define MAXSTACKSIZE 16

/* Static variables */
static yadsl_BigIntHandle* stack[MAXSTACKSIZE+1];
static int top;

/* Static functions */
static yadsl_BigIntHandle* pop();
static void popx();
static void checkstack();
static void checknull(yadsl_BigIntHandle* bigint);
static void push(yadsl_BigIntHandle* bigint);
static void checkindex(int index);
static void checkop(int opcnt);
static yadsl_BigIntHandle* at(int index);
static intmax_t get(int index);

const char *yadsl_tester_help_strings[] = {0};

yadsl_TesterRet yadsl_tester_init()
{
	return YADSL_TESTER_RET_OK;
}

static yadsl_BigIntHandle* pop()
{
	yadsl_tester_assertx(top > 0, "empty stack");
	return stack[--top];
}

static void popx()
{
	yadsl_bigint_destroy(pop());
}

static void checkstack()
{
	yadsl_tester_assertf(top <= MAXSTACKSIZE, "stack overflow", popx);
}

static void checknull(yadsl_BigIntHandle* bigint)
{
	yadsl_tester_assert(bigint != NULL, YADSL_TESTER_RET_MALLOC);
}

static void push(yadsl_BigIntHandle* bigint)
{
	checknull(bigint);
	stack[top++] = bigint;
	checkstack();
}

static void checkindex(int index)
{
	yadsl_tester_assertx(index >= 0, "index < 0");
	yadsl_tester_assertx(index < top, "index >= top");
}

static void checkop(int opcnt)
{
	yadsl_tester_assertx(top >= opcnt-1, "too few operands");
}

static yadsl_BigIntHandle* at(int index)
{
	checkindex(index);
	return stack[index];
}

static intmax_t get(int index)
{
	intmax_t i;
	yadsl_tester_assert(yadsl_bigint_to_int(at(index), &i), YADSL_TESTER_RET_OVERFLOW);
	return i;
}

static intmax_t _bigint_push()
{
	intmax_t i;
	yadsl_tester_parse_n_arguments("I", &i);
	push(yadsl_bigint_from_int(i));
	return i;
}

static void bigint_push()
{
	_bigint_push();
}

static void bigint_pop()
{
	popx();
}

static void bigint_settop()
{
	int n;
	yadsl_tester_parse_n_arguments("i", &n);
	checkindex(n);
	while (top != n) popx();
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
	intmax_t i;
	yadsl_tester_parse_n_arguments("iI", &n, &i);
	yadsl_tester_asserteqI(i, get(n), NULL);
}

static void bigint_roundtrip()
{
	intmax_t i;
	i = _bigint_push();
	yadsl_tester_asserteqI(i, get(top-1), NULL);
	popx();
}

static void bigint_copy()
{
	int n;
	yadsl_tester_parse_n_arguments("i", &n);
	push(yadsl_bigint_copy(at(n)));
}

static void bigint_copyroundtrip()
{
	intmax_t i, j;
	yadsl_BigIntHandle* copy;
	i = _bigint_push();
	copy = yadsl_bigint_copy(at(top-1));
	popx();
	push(copy);
	j = get(top-1);
	popx();
	yadsl_tester_asserteqI(i, j, NULL);
}

static void bigint_opposite()
{
	checkop(1);
	push(yadsl_bigint_opposite(at(top-1)));
}

static void bigint_oppositeroundtrip()
{
	intmax_t i, j;
	yadsl_BigIntHandle* opposite;
	i = _bigint_push();
	opposite = yadsl_bigint_opposite(at(top-1));
	popx();
	push(opposite);
	j = get(top-1);
	popx();
	yadsl_tester_asserteqI(-i, j, NULL);
}

static void bigint_add()
{
	yadsl_BigIntHandle *bigint;
	checkop(2);
	bigint = yadsl_bigint_add(at(top-2), at(top-1));
	checknull(bigint);
	popx();
	popx();
	push(bigint);
}

static void bigint_addip()
{
	intmax_t a, b, c;
	a = _bigint_push();
	b = _bigint_push();
	yadsl_tester_parse_n_arguments("I", &c);
	yadsl_tester_asserteqI(c, a + b, "parameters don't satisfy addition");
	bigint_add();
	yadsl_tester_asserteqI(c, get(top-1), "obtained and expected addition results differ");
	popx();
}

static void bigint_sub()
{
	yadsl_BigIntHandle *bigint;
	checkop(2);
	bigint = yadsl_bigint_subtract(at(top-2), at(top-1));
	checknull(bigint);
	popx();
	popx();
	push(bigint);
}

static void bigint_subip()
{
	intmax_t a, b, c;
	a = _bigint_push();
	b = _bigint_push();
	yadsl_tester_parse_n_arguments("I", &c);
	yadsl_tester_asserteqI(c, a - b, "parameters don't satisfy subtraction");
	bigint_sub();
	yadsl_tester_asserteqI(c, get(top-1), "obtained and expected subtraction results differ");
	popx();
}

#define CMD(name) { #name, bigint_ ## name }

static yadsl_TesterUtilsCommand commands[] = {
	CMD(add),
	CMD(get),
	CMD(pop),
	CMD(sub),
	CMD(copy),
	CMD(push),
	CMD(addip),
	CMD(subip),
	CMD(gettop),
	CMD(settop),
	CMD(opposite),
	CMD(roundtrip),
	CMD(copyroundtrip),
	CMD(oppositeroundtrip),
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
