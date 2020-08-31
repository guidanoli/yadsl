#include <set/set.h>

#include <yadsl/posixstring.h>
#include <stdio.h>
#include <stdlib.h>

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

yadsl_TesterRet convertReturn(SetRet setId)
{
	switch (setId) {
	case SET_OK:
		return YADSL_TESTER_RET_OK;
	case SET_MEMORY:
		return yadsl_tester_return_external_value("malloc");
	case SET_CONTAINS:
		return yadsl_tester_return_external_value("contains");
	case SET_DOES_NOT_CONTAIN:
		return yadsl_tester_return_external_value("containsnot");
	case SET_EMPTY:
		return yadsl_tester_return_external_value("empty");
	case SET_OUT_OF_BOUNDS:
		return yadsl_tester_return_external_value("bounds");
	default:
		return yadsl_tester_return_external_value("unknown");
	}
}

/* Set object */
static Set *pSet = NULL;
static char *savedStr = NULL;

static char buffer[BUFSIZ], arg[BUFSIZ];

yadsl_TesterRet yadsl_tester_init()
{
	SetRet setId;
	if (setId = setCreate(&pSet))
		return convertReturn(setId);
	return YADSL_TESTER_RET_OK;
}

int filterItem(void *item, void *arg)
{
	return matches((char *) item, (char *) arg);
}

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
	SetRet setId = SET_OK;
	char *temp;
	if matches(command, "save") {
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if ((temp = strdup(buffer)) == NULL)
			return YADSL_TESTER_RET_MALLOC;
		if (setContainsItem(pSet, savedStr) != SET_CONTAINS)
			if (savedStr)
				free(savedStr);
		savedStr = temp;
	} else if matches(command, "contains") {
		int expected, actual;
		if (yadsl_tester_parse_arguments("s", arg) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		expected = TesterUtilsGetYesOrNoFromString(arg);
		if (savedStr == NULL)
			yadsl_tester_log("Found no variable saved. Checking if contains NULL.");
		setId = setContainsItem(pSet, savedStr);
		actual = (setId == SET_CONTAINS);
		if (actual != expected)
			return YADSL_TESTER_RET_RETURN;
		else
			setId = SET_OK;
	} else if matches(command, "filter") {
		int actual, expected;
		char *foundStr;
		if (yadsl_tester_parse_arguments("ss", buffer, arg) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		if ((temp = strdup(buffer)) == NULL)
			return YADSL_TESTER_RET_MALLOC;
		expected = TesterUtilsGetYesOrNoFromString(arg);
		setId = setFilterItem(pSet, filterItem, temp, &foundStr);
		free(temp);
		actual = (setId == SET_OK);
		if (actual != expected)
			return YADSL_TESTER_RET_RETURN;
		else
			setId = SET_OK;
	} else if matches(command, "filtersave") {
		char *foundStr;
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if ((temp = strdup(buffer)) == NULL)
			return YADSL_TESTER_RET_MALLOC;
		setId = setFilterItem(pSet, filterItem, temp, &foundStr);
		if (setId == SET_OK) {
			if (savedStr != NULL &&
				setContainsItem(pSet, savedStr) != SET_CONTAINS)
				free(savedStr);
			savedStr = foundStr;
		}
		free(temp);
	} else if matches(command, "add") {
		if (savedStr == NULL)
			yadsl_tester_log("Found no variable saved. Adding NULL.");
		setId = setAddItem(pSet, savedStr);
	} else if matches(command, "remove") {
		if (savedStr == NULL)
			yadsl_tester_log("Found no variable saved. Removing NULL.");
		setId = setRemoveItem(pSet, savedStr);
	} else if matches(command, "current") {
		char *currentStr;
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if ((temp = strdup(buffer)) == NULL)
			return YADSL_TESTER_RET_MALLOC;
		if (setId = setGetCurrentItem(pSet, &currentStr)) {
			free(temp);
		} else {
			int equal;
			if (currentStr == NULL) {
				free(temp);
				yadsl_tester_log("The current item is NULL");
				return YADSL_TESTER_RET_RETURN;
			}
			equal = matches(temp, currentStr);
			free(temp);
			if (!equal) {
				yadsl_tester_log("%s is the current item", currentStr);
				return YADSL_TESTER_RET_RETURN;
			}
		}
	} else if matches(command, "size") {
		size_t expected, actual;
		if (yadsl_tester_parse_arguments("z", &expected) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		setId = setGetSize(pSet, &actual);
		if (setId == SET_OK && actual != expected)
			return YADSL_TESTER_RET_RETURN;
	} else if matches(command, "previous") {
		setId = setPreviousItem(pSet);
	} else if matches(command, "next") {
		setId = setNextItem(pSet);
	} else if matches(command, "first") {
		setId = setFirstItem(pSet);
	} else if matches(command, "last") {
		setId = setLastItem(pSet);
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
	setDestroyDeep(pSet, freeItem, NULL);
	if (savedStr != NULL)
		free(savedStr);
	return YADSL_TESTER_RET_OK;
}
