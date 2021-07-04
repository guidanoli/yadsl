#include <avl/avl.h>

#include <assert.h>

#include <tester/tester.h>
#include <testerutils/testerutils.h>

const char *yadsl_tester_help_strings[] = {
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
yadsl_AVLTreeHandle *pTree;
yadsl_TesterRet cbReturnValue;
int first, last;

static int cmp_objs_func(void *obj1, void *obj2, void *cmp_objs_arg)
{
	assert(cmp_objs_arg == &pTree);
	return *((int *) obj1) - *((int *) obj2);
}

static void *my_ranged_visit_cb(void *object, void *cmp_objs_arg)
{
	int curr;
	assert(object);
	assert(cmp_objs_arg == &pTree);
	curr = *((int *) object);
	if (curr != first)
		cbReturnValue = YADSL_TESTER_RET_RETURN;
	else
		if (first < last)
			++first;
		else if (first > last)
			--first;
	return 0;
}

static void *my_visit_cb(void *object, void *cmp_objs_arg)
{
	int actual, expected;
	assert(object);
	assert(cmp_objs_arg == &pTree);
	actual = *((int *) object);
	if (yadsl_tester_parse_arguments("i", &expected) != 1) {
		if (!cbReturnValue)
			cbReturnValue = YADSL_TESTER_RET_ARGUMENT;
		return 0;
	}
	if (!cbReturnValue && actual != expected)
		cbReturnValue = YADSL_TESTER_RET_RETURN;
	return 0;
}

static void my_free(void* obj, void* arg)
{
	free(obj);
}

yadsl_TesterRet yadsl_tester_init()
{
	if (!(pTree = yadsl_avltree_tree_create()))
		return YADSL_TESTER_RET_MALLOC;
	return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet convert(yadsl_AVLTreeRet returnId)
{
	switch (returnId) {
	case YADSL_AVLTREE_RET_OK:
		return YADSL_TESTER_RET_OK;
	case YADSL_AVLTREE_RET_MEMORY:
		return YADSL_TESTER_RET_MALLOC;
	default:
		return yadsl_tester_error("unknown");
	}
}

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
	yadsl_AVLTreeRet returnId = YADSL_AVLTREE_RET_OK;
	if (yadsl_testerutils_match(command, "new")) {
		yadsl_AVLTreeHandle *newTree;
		if (!(newTree = yadsl_avltree_tree_create())) {
			return YADSL_TESTER_RET_MALLOC;
		} else {
			yadsl_AVLTreeCallbacks callbacks = {.free_cb = my_free};
			yadsl_avltree_destroy(pTree, &callbacks);
			pTree = newTree;
		}
	} else if (yadsl_testerutils_match(command, "insert")) {
		int* pNumber;
		int number;
		bool actual, expected;
		yadsl_AVLTreeCallbacks callbacks = {.compare_cb = cmp_objs_func, .compare_arg = &pTree};
		if (yadsl_tester_parse_arguments("is", &number, buffer) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		pNumber = malloc(sizeof(int));
		if (pNumber == NULL)
			return YADSL_TESTER_RET_MALLOC;
		*pNumber = number;
		expected = !yadsl_testerutils_str_to_bool(buffer);
		returnId = yadsl_avltree_object_insert(pTree, pNumber, &callbacks, &actual);
		if (returnId || actual)
			free(pNumber);
		if (!returnId && actual != expected)
			return YADSL_TESTER_RET_RETURN;
	} else if (yadsl_testerutils_match(command, "insert*")) {
		int first, last;
		bool expected, actual;
		yadsl_AVLTreeCallbacks callbacks = {.compare_cb = cmp_objs_func, .compare_arg = &pTree};
		if (yadsl_tester_parse_arguments("iis", &first, &last, buffer) != 3)
			return YADSL_TESTER_RET_ARGUMENT;
		expected = !yadsl_testerutils_str_to_bool(buffer);
		do {
			int* pNumber;
			pNumber = malloc(sizeof(int));
			if (pNumber == NULL)
				return YADSL_TESTER_RET_MALLOC;
			*pNumber = first;
			returnId = yadsl_avltree_object_insert(pTree, pNumber, &callbacks, &actual);
			if (returnId || actual) {
				free(pNumber);
				break;
			}
			if (!returnId && actual != expected)
				return YADSL_TESTER_RET_RETURN;
			if (first < last)
				++first;
			else if (first > last)
				--first;
		} while (first != last);
	} else if (yadsl_testerutils_match(command, "contains")) {
		int *pNumber;
		int number;
		bool actual, expected;
		yadsl_AVLTreeCallbacks callbacks = {.compare_cb = cmp_objs_func, .compare_arg = &pTree};
		if (yadsl_tester_parse_arguments("is", &number, buffer) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		pNumber = malloc(sizeof(int));
		if (pNumber == NULL)
			return YADSL_TESTER_RET_MALLOC;
		*pNumber = number;
		expected = yadsl_testerutils_str_to_bool(buffer);
		returnId = yadsl_avltree_object_search(pTree, pNumber, &callbacks, &actual);
		free(pNumber);
		if (!returnId && actual != expected)
			return YADSL_TESTER_RET_RETURN;
	} else if (yadsl_testerutils_match(command, "contains*")) {
		int first, last;
		bool expected, actual;
		yadsl_AVLTreeCallbacks callbacks = {.compare_cb = cmp_objs_func, .compare_arg = &pTree};
		if (yadsl_tester_parse_arguments("iis", &first, &last, buffer) != 3)
			return YADSL_TESTER_RET_ARGUMENT;
		expected = yadsl_testerutils_str_to_bool(buffer);
		do {
			int* pNumber;
			pNumber = malloc(sizeof(int));
			if (pNumber == NULL)
				return YADSL_TESTER_RET_MALLOC;
			*pNumber = first;
			returnId = yadsl_avltree_object_search(pTree, pNumber, &callbacks, &actual);
			free(pNumber);
			if (returnId)
				break;
			else if (actual != expected)
				return YADSL_TESTER_RET_RETURN;
			if (first < last)
				++first;
			else if (first > last)
				--first;
		} while (first != last);
	} else if (yadsl_testerutils_match(command, "traverse")) {
		cbReturnValue = YADSL_AVLTREE_RET_OK;
		yadsl_AVLTreeCallbacks callbacks = {.visit_cb = my_visit_cb, .visit_arg = &pTree};
		returnId = yadsl_avltree_tree_traverse(pTree, YADSL_AVLTREE_VISITING_IN_ORDER, &callbacks, NULL);
		if (!returnId)
			returnId = cbReturnValue;
	} else if (yadsl_testerutils_match(command, "traverse*")) {
		cbReturnValue = YADSL_AVLTREE_RET_OK;
		yadsl_AVLTreeCallbacks callbacks = {.visit_cb = my_ranged_visit_cb, .visit_arg = &pTree};
		if (yadsl_tester_parse_arguments("ii", &first, &last) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		returnId = yadsl_avltree_tree_traverse(pTree, YADSL_AVLTREE_VISITING_IN_ORDER, &callbacks, NULL);
		if (!returnId)
			returnId = cbReturnValue;
	} else if (yadsl_testerutils_match(command, "delete")) {
		int* pNumber;
		int number;
		bool expected, actual;
		yadsl_AVLTreeCallbacks callbacks = {.compare_cb = cmp_objs_func, .compare_arg = &pTree, .free_cb = my_free};
		if (yadsl_tester_parse_arguments("is", &number, buffer) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		pNumber = malloc(sizeof(int));
		if (pNumber == NULL)
			return YADSL_TESTER_RET_MALLOC;
		*pNumber = number;
		expected = yadsl_testerutils_str_to_bool(buffer);
		returnId = yadsl_avltree_object_remove(pTree, pNumber, &callbacks, &actual);
		free(pNumber);
		if (!returnId && actual != expected)
			return YADSL_TESTER_RET_RETURN;
	} else if (yadsl_testerutils_match(command, "delete*")) {
		int first, last;
		bool expected, actual;
		yadsl_AVLTreeCallbacks callbacks = {.compare_cb = cmp_objs_func, .compare_arg = &pTree, .free_cb = my_free};
		if (yadsl_tester_parse_arguments("iis", &first, &last, buffer) != 3)
			return YADSL_TESTER_RET_ARGUMENT;
		expected = yadsl_testerutils_str_to_bool(buffer);
		do {
			int *pNumber;
			pNumber = malloc(sizeof(int));
			if (pNumber == NULL)
				return YADSL_TESTER_RET_MALLOC;
			*pNumber = first;
			returnId = yadsl_avltree_object_remove(pTree, pNumber, &callbacks, &actual);
			free(pNumber);
			if (returnId)
				break;
			else if (actual != expected)
				return YADSL_TESTER_RET_RETURN;
			if (first < last)
				++first;
			else if (first > last)
				--first;
		} while (first != last);
	} else {
		return YADSL_TESTER_RET_COMMAND;
	}
	return convert(returnId);
}

yadsl_TesterRet yadsl_tester_release()
{
	yadsl_AVLTreeCallbacks callbacks = {.free_cb = my_free};
	if (pTree)
		yadsl_avltree_destroy(pTree, &callbacks);
	return YADSL_TESTER_RET_OK;
}

