#include <set/set.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <string/string.h>
#include <tester/tester.h>
#include <testerutils/testerutils.h>

/* Help */

const char *yadsl_tester_help_strings[] = {
	"This is an interactive module of the set library",
	"You interact with a single set object at all times",
	"",
	"The registered actions are the following:",
	"/save <string>             save string",
	"/contains [YES/NO]         check if set contains saved string",
	"/filter <string> [YES/NO]  check if filter matches one string",
	"/filtersave <string>       filter string and save it",
	"/add                       add saved string to set",
	"/remove                    remove saved string from set",
	"/current <expected>        get string pointed by the cursor",
	"/size <expected>           get set size",
	"/previous                  move cursor to previous string",
	"/next                      move cursor to next string",
	"/first                     move cursor to first string",
	"/last                      move cursor to last string",
	NULL, /* Sentinel */
};

yadsl_TesterRet convertReturn(yadsl_SetRet setId)
{
	switch (setId) {
	case YADSL_SET_RET_OK:
		return YADSL_TESTER_RET_OK;
	case YADSL_SET_RET_MEMORY:
		return yadsl_tester_return_external_value("malloc");
	case YADSL_SET_RET_CONTAINS:
		return yadsl_tester_return_external_value("contains");
	case YADSL_SET_RET_DOES_NOT_CONTAIN:
		return yadsl_tester_return_external_value("containsnot");
	case YADSL_SET_RET_EMPTY:
		return yadsl_tester_return_external_value("empty");
	case YADSL_SET_RET_OUT_OF_BOUNDS:
		return yadsl_tester_return_external_value("bounds");
	default:
		return yadsl_tester_return_external_value("unknown");
	}
}

/* Set object */
static yadsl_SetHandle *pSet = NULL;
static char *savedStr = NULL;

static char buffer[BUFSIZ], arg[BUFSIZ];

yadsl_TesterRet yadsl_tester_init()
{
	if (pSet = yadsl_set_create())
		return YADSL_TESTER_RET_OK;
	else
		return YADSL_TESTER_RET_MALLOC;
}

bool filterItem(void *item, void *arg)
{
	return strcmp((char *) item, (char *) arg) == 0;
}

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
	yadsl_SetRet setId = YADSL_SET_RET_OK;
	char *temp;
	if (yadsl_testerutils_match(command, "save")) {
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if ((temp = yadsl_string_duplicate(buffer)) == NULL)
			return YADSL_TESTER_RET_MALLOC;
		if (yadsl_set_item_contains_check(pSet, savedStr) != YADSL_SET_RET_CONTAINS)
			if (savedStr)
				free(savedStr);
		savedStr = temp;
	} else if (yadsl_testerutils_match(command, "contains")) {
		int expected, actual;
		if (yadsl_tester_parse_arguments("s", arg) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		expected = yadsl_testerutils_str_to_bool(arg);
		if (savedStr == NULL)
			yadsl_tester_log("Found no variable saved. Checking if contains NULL.");
		setId = yadsl_set_item_contains_check(pSet, savedStr);
		actual = (setId == YADSL_SET_RET_CONTAINS);
		if (actual != expected)
			return YADSL_TESTER_RET_RETURN;
		else
			setId = YADSL_SET_RET_OK;
	} else if (yadsl_testerutils_match(command, "filter")) {
		int actual, expected;
		char *foundStr;
		if (yadsl_tester_parse_arguments("ss", buffer, arg) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		if ((temp = yadsl_string_duplicate(buffer)) == NULL)
			return YADSL_TESTER_RET_MALLOC;
		expected = yadsl_testerutils_str_to_bool(arg);
		setId = yadsl_set_item_filter(pSet, filterItem, temp, &foundStr);
		free(temp);
		actual = (setId == YADSL_SET_RET_OK);
		if (actual != expected)
			return YADSL_TESTER_RET_RETURN;
		else
			setId = YADSL_SET_RET_OK;
	} else if (yadsl_testerutils_match(command, "filtersave")) {
		char *foundStr;
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if ((temp = yadsl_string_duplicate(buffer)) == NULL)
			return YADSL_TESTER_RET_MALLOC;
		setId = yadsl_set_item_filter(pSet, filterItem, temp, &foundStr);
		if (setId == YADSL_SET_RET_OK) {
			if (savedStr != NULL &&
				yadsl_set_item_contains_check(pSet, savedStr) != YADSL_SET_RET_CONTAINS)
				free(savedStr);
			savedStr = foundStr;
		}
		free(temp);
	} else if (yadsl_testerutils_match(command, "add")) {
		if (savedStr == NULL)
			yadsl_tester_log("Found no variable saved. Adding NULL.");
		setId = yadsl_set_item_add(pSet, savedStr);
	} else if (yadsl_testerutils_match(command, "remove")) {
		if (savedStr == NULL)
			yadsl_tester_log("Found no variable saved. Removing NULL.");
		setId = yadsl_set_item_remove(pSet, savedStr);
	} else if (yadsl_testerutils_match(command, "current")) {
		char *currentStr;
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if ((temp = yadsl_string_duplicate(buffer)) == NULL)
			return YADSL_TESTER_RET_MALLOC;
		if (setId = yadsl_set_cursor_get(pSet, &currentStr)) {
			free(temp);
		} else {
			int equal;
			if (currentStr == NULL) {
				free(temp);
				yadsl_tester_log("The current item is NULL");
				return YADSL_TESTER_RET_RETURN;
			}
			equal = strcmp(temp, currentStr) == 0;
			free(temp);
			if (!equal) {
				yadsl_tester_log("%s is the current item", currentStr);
				return YADSL_TESTER_RET_RETURN;
			}
		}
	} else if (yadsl_testerutils_match(command, "size")) {
		size_t expected, actual;
		if (yadsl_tester_parse_arguments("z", &expected) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		setId = yadsl_set_size_get(pSet, &actual);
		if (setId == YADSL_SET_RET_OK && actual != expected)
			return YADSL_TESTER_RET_RETURN;
	} else if (yadsl_testerutils_match(command, "previous")) {
		setId = yadsl_set_cursor_previous(pSet);
	} else if (yadsl_testerutils_match(command, "next")) {
		setId = yadsl_set_cursor_next(pSet);
	} else if (yadsl_testerutils_match(command, "first")) {
		setId = yadsl_set_cursor_first(pSet);
	} else if (yadsl_testerutils_match(command, "last")) {
		setId = yadsl_set_cursor_last(pSet);
	} else {
		return YADSL_TESTER_RET_COMMAND;
	}
	return convertReturn(setId);
}

void freeItem(void *item, void *arg)
{
	char *str = (char *) item;
	if (str == savedStr)
		savedStr = NULL;
	free(str);
}

yadsl_TesterRet yadsl_tester_release()
{
	yadsl_set_destroy(pSet, freeItem, NULL);
	if (savedStr != NULL)
		free(savedStr);
	return YADSL_TESTER_RET_OK;
}
