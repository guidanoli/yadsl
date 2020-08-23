#include <stack/stack.h>

#include <yadsl/posixstring.h>
#include <stdlib.h>
#include <stdio.h>

#include <tester/tester.h>

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

static Stack *st;
char buffer[BUFSIZ];

TesterReturnValue TesterInitCallback()
{
	st = NULL;
	return TESTER_OK;
}

TesterReturnValue convert(StackRet ret)
{
	switch (ret) {
	case STACK_OK:
		return TESTER_OK;
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
	StackRet ret = STACK_OK;
	if matches(command, "create") {
		ret = stackCreate(&st);
	} else if matches(command, "destroy") {
		stackDestroy(st, free);
		st = NULL;
	} else if matches(command, "add") {
		int num, *num_ptr;
		if (TesterParseArguments("i", &num) != 1)
			return TESTER_ARGUMENT;
		num_ptr = malloc(sizeof(int));
		if (num_ptr == NULL)
			return TESTER_MALLOC;
		*num_ptr = num;
		ret = stackAdd(st, num_ptr);
		if (ret)
			free(num_ptr);
	} else if matches(command, "remove") {
		int *actual_ptr, expected;
		if (TesterParseArguments("i", &expected) != 1)
			return TESTER_ARGUMENT;
		ret = stackRemove(st, &actual_ptr);
		if (!ret) {
			int actual;
			actual = *actual_ptr;
			free(actual_ptr);
			if (expected != actual)
				return TESTER_ARGUMENT;
		}
	} else if matches(command, "empty") {
		int actual, expected;
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_ARGUMENT;
		expected = matches(buffer, "YES");
		if (!expected && !matches(buffer, "NO"))
			TesterLog("Argument wasn't YES nor NO, so 'NO' was assumed.");
		ret = stackEmpty(st, &actual);
		if ((ret == STACK_OK || ret == STACK_EMPTY) && (actual != expected))
			return TESTER_RETURN;
	} else {
		return TESTER_COMMAND;
	}
	return convert(ret);
}

TesterReturnValue TesterExitCallback()
{
	if (st) stackDestroy(st, free);
	return TESTER_OK;
}

