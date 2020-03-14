#include "stack.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "tester.h"

#define matches(a, b) (strcmp(a, b) == 0)

const char *TesterHelpStrings[] = {
	"This is the stack test module",
	"/create",
	"/destroy",
	"/add <number>",
	"/remove <expected number>",
	"/empty [YES/NO]",
	NULL
};

static stack *st;
char buffer[BUFSIZ];

TesterReturnValue TesterInitCallback()
{
	st = NULL;
	return TESTER_RETURN_OK;
}

TesterReturnValue convert(stack_return ret)
{
	switch (ret) {
	case STACK_OK:
		return TESTER_RETURN_OK;
	case STACK_EMPTY:
		return TesterExternalReturnValue("empty");
	case STACK_MEMORY:
		return TesterExternalReturnValue("memory");
	default:
		return TesterExternalReturnValue("unknown");
	}
}

TesterReturnValue TesterParseCallback(const char *command)
{
	stack_return ret = STACK_OK;
	if matches(command, "create") {
		ret = stack_create(&st);
	} else if matches(command, "destroy") {
		ret = stack_destroy(st, free);
		if (!ret)
			st = NULL;
	} else if matches(command, "add") {
		int num, *num_ptr;
		if (TesterParseArguments("i", &num) != 1)
			return TESTER_RETURN_ARGUMENT;
		num_ptr = malloc(sizeof(int));
		if (num_ptr == NULL)
			return TESTER_RETURN_MALLOC;
		*num_ptr = num;
		ret = stack_add(st, num_ptr);
		if (ret)
			free(num_ptr);
	} else if matches(command, "remove") {
		int *actual_ptr, expected;
		if (TesterParseArguments("i", &expected) != 1)
			return TESTER_RETURN_ARGUMENT;
		ret = stack_remove(st, &actual_ptr);
		if (!ret) {
			int actual;
			actual = *actual_ptr;
			free(actual_ptr);
			if (expected != actual)
				return TESTER_RETURN_ARGUMENT;
		}
	} else if matches(command, "empty") {
		int actual, expected;
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_RETURN_ARGUMENT;
		expected = matches(buffer, "YES");
		if (!expected && !matches(buffer, "NO"))
			TesterLog("Argument wasn't YES nor NO, so 'NO' was assumed.");
		ret = stack_empty(st, &actual);
		if ((ret == STACK_OK || ret == STACK_EMPTY) && (actual != expected))
			return TESTER_RETURN_RETURN;
	} else {
		return TESTER_RETURN_COMMAND;
	}
	return convert(ret);
}

TesterReturnValue TesterExitCallback()
{
	if (st) return convert(stack_destroy(st, free));
	return TESTER_RETURN_OK;
}

