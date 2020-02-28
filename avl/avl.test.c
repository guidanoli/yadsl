#include "avl.h"

#include <string.h>

#include <common/assert.h>

#include "tester.h"

#define matches(a,b) (strcmp(a,b) == 0)

const char *TesterHelpStrings[] = {
	"This is the avl test module",
	"A new pTree is already created in the beggining",
	"",
	"/new",
	"/insert <number>",
	"/contains <number> [YES/NO]",
};

char buffer[BUFSIZ];
AVLTree *pTree;

int cmpObjs(void *obj1, void *obj2, void *arg) {
	_assert(arg == &pTree);
	return *((int *) obj1) - *((int *) obj2);
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

