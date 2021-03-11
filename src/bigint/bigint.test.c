#include <bigint/bigint.h>

#include <tester/tester.h>
#include <testerutils/testerutils.h>

#include <stddef.h>
#include <inttypes.h>

#define MAXSTACKSIZE 16

/* Static variables */
static yadsl_BigIntHandle* stack[MAXSTACKSIZE+1];
static char* s;
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
	yadsl_tester_assertf(top > 0, "empty stack");
	return stack[--top];
}

static void bigintcheck()
{
	yadsl_BigIntStatus status = yadsl_bigint_check(at(top-1));
	switch (status) {
	case YADSL_BIGINT_STATUS_OK:
		break;
	case YADSL_BIGINT_STATUS_INVALID_SIZE:
		yadsl_tester_throwf("CHECK: Invalid size");
		break;
	case YADSL_BIGINT_STATUS_INVALID_DIGITS:
		yadsl_tester_throwf("CHECK: Invalid digits");
		break;
	case YADSL_BIGINT_STATUS_LEADING_ZEROS:
		yadsl_tester_throwf("CHECK: Leading zeros");
		break;
	default:
		yadsl_tester_throwf("CHECK: Unknown status code '%d'", status);
	}
}

static void popx()
{
	yadsl_bigint_destroy(pop());
}

static void checkstack()
{
	yadsl_tester_xassertf(top <= MAXSTACKSIZE, "stack overflow", popx);
}

static void checknull(void *p)
{
	yadsl_tester_assert(p != NULL, YADSL_TESTER_RET_MALLOC);
}

static void push(yadsl_BigIntHandle* bigint)
{
	checknull(bigint);
	stack[top++] = bigint;
	checkstack();
	bigintcheck();
}

static void freestr()
{
	if (s != NULL) {
		free(s);
		s = NULL;
	}
}

static void checkindex(int index)
{
	yadsl_tester_assertf(index >= 0, "index < 0");
	yadsl_tester_assertf(index < top, "index >= top");
}

static void checkop(int opcnt)
{
	yadsl_tester_assertf(top >= opcnt-1, "too few operands");
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

static void check_to_int(intmax_t i, int index)
{
	yadsl_tester_asserteqI(i, get(index), "check_to_int failed");
}

static void check_to_string(intmax_t i, int index)
{
	intmax_t j;
	int scanned;
	freestr();
	s = yadsl_bigint_to_string(at(index));
	checknull(s);
	scanned = sscanf(s, "%" SCNdMAX, &j);
	yadsl_tester_assertf(scanned == 1, "sscanf returned %d", scanned);
	yadsl_tester_asserteqI(i, j, "string check failed");
}

static void check_copy(intmax_t i, int index)
{
	intmax_t j;
	int cmp;
	push(yadsl_bigint_copy(at(index)));
	cmp = yadsl_bigint_compare(at(index), at(top-1));
	yadsl_tester_asserteqi(cmp, 0, "copy check failed");
	j = get(top-1);
	popx();
	yadsl_tester_asserteqI(i, j, "copy check failed");

}

static void check_opposite(intmax_t i, int index)
{
	intmax_t j;
	push(yadsl_bigint_opposite(at(index)));
	j = get(top-1);
	popx();
	yadsl_tester_asserteqI(-i, j, "opposite check failed");
}

static intmax_t _bigint_push()
{
	intmax_t i, j;
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

static void bigint_check()
{
	intmax_t i, j;
	i = _bigint_push();
	check_to_int(i, top-1);
	check_to_string(i, top-1);
	check_copy(i, top-1);
	check_opposite(i, top-1);
	popx();
}

static void bigint_copy()
{
	int n;
	yadsl_tester_parse_n_arguments("i", &n);
	push(yadsl_bigint_copy(at(n)));
}

static void bigint_opposite()
{
	checkop(1);
	push(yadsl_bigint_opposite(at(top-1)));
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

static void bigint_cmp()
{
	intmax_t a, b;
	char op;
	int cmp;
	a = _bigint_push();
	yadsl_tester_parse_n_arguments("c", &op);
	b = _bigint_push();
	cmp = yadsl_bigint_compare(at(top-2), at(top-1));
	popx();
	popx();
	switch (op) {
	case '>':
		yadsl_tester_assertf(cmp > 0, "%" PRIdMAX " <= %" PRIdMAX, a, b);
		break;
	case '<':
		yadsl_tester_assertf(cmp < 0, "%" PRIdMAX " >= %" PRIdMAX, a, b);
		break;
	case '=':
		yadsl_tester_assertf(cmp == 0, "%" PRIdMAX " != %" PRIdMAX, a, b);
		break;
	default:
		yadsl_tester_throwf("'%c' is not a valid operator", op);
	}
}

#define CMD(name) { #name, bigint_ ## name }

static yadsl_TesterUtilsCommand commands[] = {
	CMD(add),
	CMD(cmp),
	CMD(get),
	CMD(pop),
	CMD(sub),
	CMD(copy),
	CMD(push),
	CMD(addip),
	CMD(check),
	CMD(subip),
	CMD(gettop),
	CMD(settop),
	CMD(opposite),
	{ NULL, NULL },
};

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
	return yadsl_testerutils_parse_command(command, commands);
}

yadsl_TesterRet yadsl_tester_release()
{
	freestr();
	while (top != 0)
		yadsl_bigint_destroy(pop());
	return YADSL_TESTER_RET_OK;
}
