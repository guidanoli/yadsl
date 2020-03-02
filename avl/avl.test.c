#include "avl.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tester.h"

#define matches(a,b) (strcmp(a,b) == 0)

const char *TesterHelpStrings[] = {
	"This is the avl test module",
	"A new pTree is already created in the beggining",
	"",
	"/new                                 create a new tree",
	"/insert <number> [YES/NO]            insert number in tree",
	"/insert* <first> <last> [YES/NO]     insert from first to last in tree",
	"/contains <number> [YES/NO]          check if number is in tree",
	"/contains* <first> <last> [YES/NO]   check from first to last in tree",
	"/traverse <numbers..>                traverse the tree in cresc. order",
	"/traverse* <first> <last>            traverse from first to last ...",
	"/delete <number> [YES/NO]            delete number from tree",
	"/delete* <first> <last> [YES/NO]     delete from first to last ...",
	NULL,
};

char buffer[BUFSIZ];
AVL *pTree;
TesterReturnValue cbReturnValue;
int first, last;

int cmpObjs(void *obj1, void *obj2, void *arg)
{
	assert(arg == &pTree);
	return *((int *) obj1) - *((int *) obj2);
}

void *visit_cb_range(void *object, void *arg)
{
	int curr;
	assert(object);
	assert(arg == &pTree);
	curr = *((int *) object);
	if (curr != first)
		cbReturnValue = TESTER_RETURN_RETURN;
	else
		if (first < last)
			++first;
		else if (first > last)
			--first;
	return 0;
}

void *visit_cb(void *object, void *arg)
{
	int actual, expected;
	assert(object);
	assert(arg == &pTree);
	actual = *((int *) object);
	if (TesterParseArguments("i", &expected) != 1) {
		if (!cbReturnValue)
			cbReturnValue = TESTER_RETURN_ARGUMENT;
		return 0;
	}
	if (!cbReturnValue && actual != expected)
		cbReturnValue = TESTER_RETURN_RETURN;
	return 0;
}

TesterReturnValue TesterInitCallback()
{
	if (avlCreate(&pTree, cmpObjs, free, &pTree))
		return TESTER_RETURN_MALLOC;
	return TESTER_RETURN_OK;
}

TesterReturnValue convert(AVLReturnId returnId)
{
	switch (returnId) {
	case AVL_RETURN_OK:
		return TESTER_RETURN_OK;
	case AVL_RETURN_INVALID_PARAMETER:
		return TesterExternalReturnValue("invalid parameter");
	case AVL_RETURN_MEMORY:
		return TesterExternalReturnValue("memory");
	default:
		return TesterExternalReturnValue("unknown");
	}
}

int getYesOrNoFromString(const char *str)
{
	int yes = matches(str, "YES");
	if (!yes && !matches(str, "NO"))
		TesterLog("Expected YES or NO, but got %s. Assumed NO.", str);
	return yes;
}

TesterReturnValue TesterParseCallback(const char *command)
{
	AVLReturnId returnId = AVL_RETURN_OK;
	if matches(command, "new") {
		AVL *newTree;
		returnId = avlCreate(&newTree, cmpObjs, free, &pTree);
		if (!returnId) {
			avlDestroy(pTree);
			pTree = newTree;
		}
	} else if matches(command, "insert") {
		int *pNumber, actual, expected;
		pNumber = malloc(sizeof(int));
		if (pNumber == NULL)
			return TESTER_RETURN_MALLOC;
		if (TesterParseArguments("is", pNumber, buffer) != 2)
			return TESTER_RETURN_ARGUMENT;
		expected = !getYesOrNoFromString(buffer);
		returnId = avlInsert(pTree, pNumber, &actual);
		if (returnId || actual)
			free(pNumber);
		if (!returnId && actual != expected)
			return TESTER_RETURN_RETURN;
	} else if matches(command, "insert*") {
		int first, last, expected, actual;
		if (TesterParseArguments("iis", &first, &last, buffer) != 3)
			return TESTER_RETURN_ARGUMENT;
		expected = !getYesOrNoFromString(buffer);
		do {
			int *pNumber;
			pNumber = malloc(sizeof(int));
			if (pNumber == NULL)
				return TESTER_RETURN_MALLOC;
			*pNumber = first;
			returnId = avlInsert(pTree, pNumber, &actual);
			if (returnId || actual) {
				free(pNumber);
				break;
			}
			if (!returnId && actual != expected)
				return TESTER_RETURN_RETURN;
			if (first < last)
				++first;
			else if (first > last)
				--first;
		} while (first != last);
	} else if matches(command, "contains") {
		int actual, expected, *pNumber;
		pNumber = malloc(sizeof(int));
		if (pNumber == NULL)
			return TESTER_RETURN_MALLOC;
		if (TesterParseArguments("is", pNumber, buffer) != 2)
			return TESTER_RETURN_ARGUMENT;
		expected = matches(buffer, "YES");
		if (!expected && !matches(buffer, "NO"))
			TesterLog("Expected YES or NO, but got %s. Assumed NO.", buffer);
		returnId = avlSearch(pTree, pNumber, &actual);
		free(pNumber);
		if (!returnId && actual != expected)
			return TESTER_RETURN_RETURN;
	} else if matches(command, "contains*") {
		int first, last, expected, actual;
		if (TesterParseArguments("iis", &first, &last, buffer) != 3)
			return TESTER_RETURN_ARGUMENT;
		expected = matches(buffer, "YES");
		if (!expected && !matches(buffer, "NO"))
			TesterLog("Expected YES or NO, but got %s. Assumed NO.", buffer);
		do {
			int *pNumber;
			pNumber = malloc(sizeof(int));
			if (pNumber == NULL)
				return TESTER_RETURN_MALLOC;
			*pNumber = first;
			returnId = avlSearch(pTree, pNumber, &actual);
			free(pNumber);
			if (returnId)
				break;
			else if (actual != expected)
				return TESTER_RETURN_RETURN;
			if (first < last)
				++first;
			else if (first > last)
				--first;
		} while (first != last);
	} else if matches(command, "traverse") {
		cbReturnValue = AVL_RETURN_OK;
		returnId = avlTraverse(pTree, visit_cb, &pTree, NULL);
		if (!returnId)
			returnId = cbReturnValue;
	} else if matches(command, "traverse*") {
		cbReturnValue = AVL_RETURN_OK;
		if (TesterParseArguments("ii", &first, &last) != 2)
			return TESTER_RETURN_ARGUMENT;
		returnId = avlTraverse(pTree, visit_cb_range, &pTree, NULL);
		if (!returnId)
			returnId = cbReturnValue;
	} else if matches(command, "delete") {
		int *pNumber, expected, actual;
		pNumber = malloc(sizeof(int));
		if (pNumber == NULL)
			return TESTER_RETURN_MALLOC;
		if (TesterParseArguments("is", pNumber, buffer) != 2)
			return TESTER_RETURN_ARGUMENT;
		expected = getYesOrNoFromString(buffer);
		returnId = avlDelete(pTree, pNumber, &actual);
		free(pNumber);
		if (!returnId && actual != expected)
			return TESTER_RETURN_RETURN;
	} else if matches(command, "delete*") {
		int first, last, expected, actual;
		if (TesterParseArguments("iis", &first, &last, buffer) != 3)
			return TESTER_RETURN_ARGUMENT;
		expected = getYesOrNoFromString(buffer);
		do {
			int *pNumber;
			pNumber = malloc(sizeof(int));
			if (pNumber == NULL)
				return TESTER_RETURN_MALLOC;
			*pNumber = first;
			returnId = avlDelete(pTree, pNumber, &actual);
			free(pNumber);
			if (returnId)
				break;
			else if (actual != expected)
				return TESTER_RETURN_RETURN;
			if (first < last)
				++first;
			else if (first > last)
				--first;
		} while (first != last);
	} else {
		return TESTER_RETURN_COMMAND;
	}
	return convert(returnId);
}

TesterReturnValue TesterExitCallback()
{
	if (pTree)
		avlDestroy(pTree);
	return TESTER_RETURN_OK;
}

