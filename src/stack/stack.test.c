#include <stack/stack.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <tester/tester.h>
#include <testerutils/testerutils.h>

const char *yadsl_tester_help_strings[] = {
	"This is the stack test module",
	"/create",
	"/destroy",
	"/add <number>",
	"/remove <expected number>",
	"/empty [YES/NO]",
	NULL
};

static yadsl_StackHandle *st;
char buffer[BUFSIZ];

yadsl_TesterRet yadsl_tester_init()
{
	st = NULL;
	return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet convert(yadsl_StackRet ret)
{
	switch (ret) {
	case YADSL_STACK_RET_OK:
		return YADSL_TESTER_RET_OK;
	case YADSL_STACK_RET_EMPTY:
		return yadsl_tester_return_external_value("empty");
	case YADSL_STACK_RET_MEMORY:
		return yadsl_tester_return_external_value("memory");
	default:
		return yadsl_tester_return_external_value("unknown");
	}
}

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
	yadsl_StackRet ret = YADSL_STACK_RET_OK;
	if (yadsl_testerutils_match(command, "create")) {
		st = yadsl_stack_create();
		if (!st)
			return YADSL_TESTER_RET_MALLOC;
	} else if (yadsl_testerutils_match(command, "destroy")) {
		yadsl_stack_destroy(st, free);
		st = NULL;
	} else if (yadsl_testerutils_match(command, "add")) {
		int num, *num_ptr;
		if (yadsl_tester_parse_arguments("i", &num) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		num_ptr = malloc(sizeof(int));
		if (num_ptr == NULL)
			return YADSL_TESTER_RET_MALLOC;
		*num_ptr = num;
		ret = yadsl_stack_item_add(st, num_ptr);
		if (ret)
			free(num_ptr);
	} else if (yadsl_testerutils_match(command, "remove")) {
		int *actual_ptr, expected;
		if (yadsl_tester_parse_arguments("i", &expected) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		ret = yadsl_stack_item_remove(st, &actual_ptr);
		if (!ret) {
			int actual;
			actual = *actual_ptr;
			free(actual_ptr);
			if (expected != actual)
				return YADSL_TESTER_RET_ARGUMENT;
		}
	} else if (yadsl_testerutils_match(command, "empty")) {
		bool actual, expected;
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		expected = yadsl_testerutils_str_to_bool(buffer);
		ret = yadsl_stack_empty_check(st, &actual);
		if ((ret == YADSL_STACK_RET_OK || ret == YADSL_STACK_RET_EMPTY) && (actual != expected))
			return YADSL_TESTER_RET_RETURN;
	} else {
		return YADSL_TESTER_RET_COMMAND;
	}
	return convert(ret);
}

yadsl_TesterRet yadsl_tester_release()
{
	if (st) yadsl_stack_destroy(st, free);
	return YADSL_TESTER_RET_OK;
}

