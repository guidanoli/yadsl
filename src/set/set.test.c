#include "set.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tester.h"
#include "testerutils.h"

/* Help */

const char *TesterHelpStrings[] = {
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

TesterReturnValue convertReturn(SetRet setId)
{
	switch (setId) {
	case SET_OK:
		return TESTER_OK;
	case SET_MEMORY:
		return TesterExternalReturnValue("malloc");
	case SET_CONTAINS:
		return TesterExternalReturnValue("contains");
	case SET_DOES_NOT_CONTAIN:
		return TesterExternalReturnValue("containsnot");
	case SET_EMPTY:
		return TesterExternalReturnValue("empty");
	case SET_OUT_OF_BOUNDS:
		return TesterExternalReturnValue("bounds");
	default:
		return TesterExternalReturnValue("unknown");
	}
}

/* Set object */
static Set *pSet = NULL;
static char *savedStr = NULL;

static char buffer[BUFSIZ], arg[BUFSIZ];

TesterReturnValue TesterInitCallback()
{
	SetRet setId;
	if (setId = setCreate(&pSet))
		return convertReturn(setId);
	return TESTER_OK;
}

int filterItem(void *item, void *arg)
{
	return matches((char *) item, (char *) arg);
}

TesterReturnValue TesterParseCallback(const char *command)
{
	SetRet setId = SET_OK;
	char *temp;
	if matches(command, "save") {
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_ARGUMENT;
		if ((temp = strdup(buffer)) == NULL)
			return TESTER_MALLOC;
		if (setContainsItem(pSet, savedStr) != SET_CONTAINS)
			if (savedStr)
				free(savedStr);
		savedStr = temp;
	} else if matches(command, "contains") {
		int expected, actual;
		if (TesterParseArguments("s", arg) != 1)
			return TESTER_ARGUMENT;
		expected = TesterGetYesOrNoFromString(arg);
		if (savedStr == NULL)
			TesterLog("Found no variable saved. Checking if contains NULL.");
		setId = setContainsItem(pSet, savedStr);
		actual = (setId == SET_CONTAINS);
		if (actual != expected)
			return TESTER_RETURN;
		else
			setId = SET_OK;
	} else if matches(command, "filter") {
		int actual, expected;
		char *foundStr;
		if (TesterParseArguments("ss", buffer, arg) != 2)
			return TESTER_ARGUMENT;
		if ((temp = strdup(buffer)) == NULL)
			return TESTER_MALLOC;
		expected = TesterGetYesOrNoFromString(arg);
		setId = setFilterItem(pSet, filterItem, temp, &foundStr);
		free(temp);
		actual = (setId == SET_OK);
		if (actual != expected)
			return TESTER_RETURN;
		else
			setId = SET_OK;
	} else if matches(command, "filtersave") {
		char *foundStr;
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_ARGUMENT;
		if ((temp = strdup(buffer)) == NULL)
			return TESTER_MALLOC;
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
			TesterLog("Found no variable saved. Adding NULL.");
		setId = setAddItem(pSet, savedStr);
	} else if matches(command, "remove") {
		if (savedStr == NULL)
			TesterLog("Found no variable saved. Removing NULL.");
		setId = setRemoveItem(pSet, savedStr);
	} else if matches(command, "current") {
		char *currentStr;
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_ARGUMENT;
		if ((temp = strdup(buffer)) == NULL)
			return TESTER_MALLOC;
		if (setId = setGetCurrentItem(pSet, &currentStr)) {
			free(temp);
		} else {
			int equal;
			if (currentStr == NULL) {
				free(temp);
				TesterLog("The current item is NULL");
				return TESTER_RETURN;
			}
			equal = matches(temp, currentStr);
			free(temp);
			if (!equal) {
				TesterLog("%s is the current item", currentStr);
				return TESTER_RETURN;
			}
		}
	} else if matches(command, "size") {
		size_t expected, actual;
		if (TesterParseArguments("z", &expected) != 1)
			return TESTER_ARGUMENT;
		setId = setGetSize(pSet, &actual);
		if (setId == SET_OK && actual != expected)
			return TESTER_RETURN;
	} else if matches(command, "previous") {
		setId = setPreviousItem(pSet);
	} else if matches(command, "next") {
		setId = setNextItem(pSet);
	} else if matches(command, "first") {
		setId = setFirstItem(pSet);
	} else if matches(command, "last") {
		setId = setLastItem(pSet);
	} else {
		return TESTER_COMMAND;
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

TesterReturnValue TesterExitCallback()
{
	setDestroyDeep(pSet, freeItem, NULL);
	if (savedStr != NULL)
		free(savedStr);
	return TESTER_OK;
}
