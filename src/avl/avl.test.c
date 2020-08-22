#include <avl/avl.h>

#include <aa/posixstring.h>
#include <stdlib.h>
#include <assert.h>

#include <tester/tester.h>

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
aa_AVLTreeHandle *pTree;
TesterReturnValue cbReturnValue;
int first, last;

int cmp_objs_func(void *obj1, void *obj2, void *cmp_objs_arg)
{
	assert(cmp_objs_arg == &pTree);
	return *((int *) obj1) - *((int *) obj2);
}

void *visit_cb_range(void *object, void *cmp_objs_arg)
{
	int curr;
	assert(object);
	assert(cmp_objs_arg == &pTree);
	curr = *((int *) object);
	if (curr != first)
		cbReturnValue = TESTER_RETURN;
	else
		if (first < last)
			++first;
		else if (first > last)
			--first;
	return 0;
}

void *visit_cb(void *object, void *cmp_objs_arg)
{
	int actual, expected;
	assert(object);
	assert(cmp_objs_arg == &pTree);
	actual = *((int *) object);
	if (TesterParseArguments("i", &expected) != 1) {
		if (!cbReturnValue)
			cbReturnValue = TESTER_ARGUMENT;
		return 0;
	}
	if (!cbReturnValue && actual != expected)
		cbReturnValue = TESTER_RETURN;
	return 0;
}

TesterReturnValue TesterInitCallback()
{
	if (aa_avltree_tree_create(cmp_objs_func, &pTree, free, &pTree))
		return TESTER_MALLOC;
	return TESTER_OK;
}

TesterReturnValue convert(aa_AVLTreeRet returnId)
{
	switch (returnId) {
	case AA_AVLTREE_RET_OK:
		return TESTER_OK;
	case AA_AVLTREE_RET_MEMORY:
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
	aa_AVLTreeRet returnId = AA_AVLTREE_RET_OK;
	if matches(command, "new") {
		aa_AVLTreeHandle *newTree;
		returnId = aa_avltree_tree_create(cmp_objs_func, &pTree, free, &newTree);
		if (!returnId) {
			aa_avltree_destroy(pTree);
			pTree = newTree;
		}
	} else if matches(command, "insert") {
		int *pNumber, actual, expected;
		pNumber = malloc(sizeof(int));
		if (pNumber == NULL)
			return TESTER_MALLOC;
		if (TesterParseArguments("is", pNumber, buffer) != 2)
			return TESTER_ARGUMENT;
		expected = !getYesOrNoFromString(buffer);
		returnId = aa_avltree_object_insert(pTree, pNumber, &actual);
		if (returnId || actual)
			free(pNumber);
		if (!returnId && actual != expected)
			return TESTER_RETURN;
	} else if matches(command, "insert*") {
		int first, last, expected, actual;
		if (TesterParseArguments("iis", &first, &last, buffer) != 3)
			return TESTER_ARGUMENT;
		expected = !getYesOrNoFromString(buffer);
		do {
			int *pNumber;
			pNumber = malloc(sizeof(int));
			if (pNumber == NULL)
				return TESTER_MALLOC;
			*pNumber = first;
			returnId = aa_avltree_object_insert(pTree, pNumber, &actual);
			if (returnId || actual) {
				free(pNumber);
				break;
			}
			if (!returnId && actual != expected)
				return TESTER_RETURN;
			if (first < last)
				++first;
			else if (first > last)
				--first;
		} while (first != last);
	} else if matches(command, "contains") {
		int actual, expected, *pNumber;
		pNumber = malloc(sizeof(int));
		if (pNumber == NULL)
			return TESTER_MALLOC;
		if (TesterParseArguments("is", pNumber, buffer) != 2)
			return TESTER_ARGUMENT;
		expected = matches(buffer, "YES");
		if (!expected && !matches(buffer, "NO"))
			TesterLog("Expected YES or NO, but got %s. Assumed NO.", buffer);
		returnId = aa_avltree_object_search(pTree, pNumber, &actual);
		free(pNumber);
		if (!returnId && actual != expected)
			return TESTER_RETURN;
	} else if matches(command, "contains*") {
		int first, last, expected, actual;
		if (TesterParseArguments("iis", &first, &last, buffer) != 3)
			return TESTER_ARGUMENT;
		expected = matches(buffer, "YES");
		if (!expected && !matches(buffer, "NO"))
			TesterLog("Expected YES or NO, but got %s. Assumed NO.", buffer);
		do {
			int *pNumber;
			pNumber = malloc(sizeof(int));
			if (pNumber == NULL)
				return TESTER_MALLOC;
			*pNumber = first;
			returnId = aa_avltree_object_search(pTree, pNumber, &actual);
			free(pNumber);
			if (returnId)
				break;
			else if (actual != expected)
				return TESTER_RETURN;
			if (first < last)
				++first;
			else if (first > last)
				--first;
		} while (first != last);
	} else if matches(command, "traverse") {
		cbReturnValue = AA_AVLTREE_RET_OK;
		returnId = aa_avltree_tree_traverse(pTree, visit_cb, &pTree, NULL);
		if (!returnId)
			returnId = cbReturnValue;
	} else if matches(command, "traverse*") {
		cbReturnValue = AA_AVLTREE_RET_OK;
		if (TesterParseArguments("ii", &first, &last) != 2)
			return TESTER_ARGUMENT;
		returnId = aa_avltree_tree_traverse(pTree, visit_cb_range, &pTree, NULL);
		if (!returnId)
			returnId = cbReturnValue;
	} else if matches(command, "delete") {
		int *pNumber, expected, actual;
		pNumber = malloc(sizeof(int));
		if (pNumber == NULL)
			return TESTER_MALLOC;
		if (TesterParseArguments("is", pNumber, buffer) != 2)
			return TESTER_ARGUMENT;
		expected = getYesOrNoFromString(buffer);
		returnId = aa_avltree_object_remove(pTree, pNumber, &actual);
		free(pNumber);
		if (!returnId && actual != expected)
			return TESTER_RETURN;
	} else if matches(command, "delete*") {
		int first, last, expected, actual;
		if (TesterParseArguments("iis", &first, &last, buffer) != 3)
			return TESTER_ARGUMENT;
		expected = getYesOrNoFromString(buffer);
		do {
			int *pNumber;
			pNumber = malloc(sizeof(int));
			if (pNumber == NULL)
				return TESTER_MALLOC;
			*pNumber = first;
			returnId = aa_avltree_object_remove(pTree, pNumber, &actual);
			free(pNumber);
			if (returnId)
				break;
			else if (actual != expected)
				return TESTER_RETURN;
			if (first < last)
				++first;
			else if (first > last)
				--first;
		} while (first != last);
	} else {
		return TESTER_COMMAND;
	}
	return convert(returnId);
}

TesterReturnValue TesterExitCallback()
{
	if (pTree)
		aa_avltree_destroy(pTree);
	return TESTER_OK;
}

