#include "avl.h"

#include <string.h>

#include <common/assert.h>

#include "tester.h"

#define matches(a,b) (strcmp(a,b) == 0)

const char *TesterHelpStrings[] = {
	"This is the avl test module",
	"A new pTree is already created in the beggining",
	"",
	"/new                                 create a new tree",
	"/insert <number>                     insert number in tree",
	"/insert* <first> <last>              insert from first to last in tree",
	"/contains <number> [YES/NO]          check if number is in tree",
	"/contains* <first> <last> [YES/NO]   check from first to last in tree",
	"/traverse <numbers..>                traverse the tree in cresc. order",
	"/traverse* <first> <last>            traverse from first to last ...",
	"/delete <number>                     delete number from tree",
	"/delete* <first> <last>              delete from first to last ...",
};

char buffer[BUFSIZ];
AVLTree *pTree;
TesterReturnValue cbReturnValue;
int first, last;

int cmpObjs(void *obj1, void *obj2, void *arg)
{
	_assert(arg == &pTree);
	return *((int *) obj1) - *((int *) obj2);
}

void visit_cb_range(void *object)
{
	int curr;
	_assert(object != NULL);
	curr = *((int *) object);
	if (curr != first)
		cbReturnValue = TESTER_RETURN_RETURN;
	else
		if (first < last)
			++first;
		else if (first > last)
			--first;
}

void visit_cb(void *object)
{
	int actual, expected;
	_assert(object != NULL);
	actual = *((int *) object);
	if (TesterParseArguments("i", &expected) != 1) {
		if (!cbReturnValue)
			cbReturnValue = TESTER_RETURN_ARGUMENT;
		return;
	}
	if (!cbReturnValue && actual != expected)
		cbReturnValue = TESTER_RETURN_RETURN;
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
	case AVL_RETURN_DUPLICATE:
		return TesterExternalReturnValue("duplicate");
	case AVL_RETURN_NODE_NOT_FOUND:
		return TesterExternalReturnValue("node not found");
	case AVL_RETURN_INVALID_PARAMETER:
		return TesterExternalReturnValue("invalid parameter");
	case AVL_RETURN_MEMORY:
		return TesterExternalReturnValue("memory");
	default:
		return TesterExternalReturnValue("unknown");
	}
}

TesterReturnValue TesterParseCallback(const char *command)
{
	AVLReturnId returnId = AVL_RETURN_OK;
	if matches(command, "new") {
		AVLTree *newTree;
		returnId = avlCreate(&newTree, cmpObjs, free, &pTree);
		if (!returnId) {
			avlDestroy(pTree);
			pTree = newTree;
		}
	} else if matches(command, "insert") {
		int *pNumber;
		pNumber = malloc(sizeof(int));
		if (pNumber == NULL)
			return TESTER_RETURN_MALLOC;
		if (TesterParseArguments("i", pNumber) != 1)
			return TESTER_RETURN_ARGUMENT;
		returnId = avlInsert(pTree, pNumber);
		if (returnId)
			free(pNumber);
	} else if matches(command, "insert*") {
		int first, last;
		if (TesterParseArguments("ii", &first, &last) != 2)
			return TESTER_RETURN_ARGUMENT;
		do {
			int *pNumber;
			pNumber = malloc(sizeof(int));
			if (pNumber == NULL)
				return TESTER_RETURN_MALLOC;
			*pNumber = first;
			returnId = avlInsert(pTree, pNumber);
			if (returnId) {
				free(pNumber);
				break;
			}
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
		returnId = avlTraverse(pTree, visit_cb);
		if (!returnId)
			returnId = cbReturnValue;
	} else if matches(command, "traverse*") {
		cbReturnValue = AVL_RETURN_OK;
		if (TesterParseArguments("ii", &first, &last) != 2)
			return TESTER_RETURN_ARGUMENT;
		returnId = avlTraverse(pTree, visit_cb_range);
		if (!returnId)
			returnId = cbReturnValue;
	} else if matches(command, "delete") {
		int *pNumber;
		pNumber = malloc(sizeof(int));
		if (pNumber == NULL)
			return TESTER_RETURN_MALLOC;
		if (TesterParseArguments("i", pNumber) != 1)
			return TESTER_RETURN_ARGUMENT;
		returnId = avlDelete(pTree, pNumber);
		free(pNumber);
	} else if matches(command, "delete*") {
		int first, last;
		if (TesterParseArguments("ii", &first, &last) != 2)
			return TESTER_RETURN_ARGUMENT;
		do {
			int *pNumber;
			pNumber = malloc(sizeof(int));
			if (pNumber == NULL)
				return TESTER_RETURN_MALLOC;
			*pNumber = first;
			returnId = avlDelete(pTree, pNumber);
			free(pNumber);
			if (returnId) break;
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

