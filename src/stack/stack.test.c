#include <stack/stack.h>

#include <yadsl/posixstring.h>
#include <stdlib.h>
#include <stdio.h>

#include <tester/tester.h>

#define matches(a, b) (strcmp(a, b) == 0)

const char *yadsl_tester_help_strings[] = {
	"This is the stack test module",
	"/create",
	"/destroy",
	"/add <number>",
	"/remove <expected number>",
	"/empty [YES/NO]",
	NULL
};

static Stack *st;
char buffer[BUFSIZ];

yadsl_TesterRet yadsl_tester_init()
{
	st = NULL;
	return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet convert(StackRet ret)
{
	switch (ret) {
	case STACK_OK:
		return YADSL_TESTER_RET_OK;
	case STACK_EMPTY:
		return yadsl_tester_return_external_value("empty");
	case STACK_MEMORY:
		return yadsl_tester_return_external_value("memory");
	default:
		return yadsl_tester_return_external_value("unknown");
	}
}

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
	StackRet ret = STACK_OK;
	if matches(command, "create") {
		ret = stackCreate(&st);
	} else if matches(command, "destroy") {
		stackDestroy(st, free);
		st = NULL;
	} else if matches(command, "add") {
		int num, *num_ptr;
		if (yadsl_tester_parse_arguments("i", &num) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		num_ptr = malloc(sizeof(int));
		if (num_ptr == NULL)
			return YADSL_TESTER_RET_MALLOC;
		*num_ptr = num;
		ret = stackAdd(st, num_ptr);
		if (ret)
			free(num_ptr);
	} else if matches(command, "remove") {
		int *actual_ptr, expected;
		if (yadsl_tester_parse_arguments("i", &expected) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		ret = stackRemove(st, &actual_ptr);
		if (!ret) {
			int actual;
			actual = *actual_ptr;
			free(actual_ptr);
			if (expected != actual)
				return YADSL_TESTER_RET_ARGUMENT;
		}
	} else if matches(command, "empty") {
		int actual, expected;
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		expected = matches(buffer, "YES");
		if (!expected && !matches(buffer, "NO"))
			yadsl_tester_log("Argument wasn't YES nor NO, so 'NO' was assumed.");
		ret = stackEmpty(st, &actual);
		if ((ret == STACK_OK || ret == STACK_EMPTY) && (actual != expected))
			return YADSL_TESTER_RET_RETURN;
	} else {
		return YADSL_TESTER_RET_COMMAND;
	}
	return convert(ret);
}

yadsl_TesterRet yadsl_tester_release()
{
	if (st) stackDestroy(st, free);
	return YADSL_TESTER_RET_OK;
}

