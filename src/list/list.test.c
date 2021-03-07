#include <list/list.h>

#include <tester/tester.h>
#include <testerutils/testerutils.h>

/* check if index is on range of C array
   (considers that index is non-negative) */
#define check_index(index, c_array) \
do { \
	if (index >= sizeof(c_array) / sizeof(*c_array)) { \
		yadsl_tester_log("Index %zu out of range (from 0 to %zu)", \
			index, sizeof(c_array) / sizeof(*c_array) - 1); \
		return YADSL_TESTER_RET_ARGUMENT; \
	} \
} while(0)

/* check if element at index in C array is not null
   (assumes index is in range) */
#define check_not_null(index, c_array) \
do { \
	if (!c_array[index]) { \
		yadsl_tester_log("Element at index %zu is NULL", index); \
		return YADSL_TESTER_RET_ARGUMENT; \
	} \
} while(0)

const char *yadsl_tester_help_strings[] = {
	"This is the interactive module of the list library",
	"You can interact with many lists at the same time",
	"Lists are identified with #, which range from 0 to 9",
	"Objects can be of any type accepted by Tester",
	"<obj> is actually <obj-type> <obj-data>",
	"All remaining lists are destroyed in the end",
	"The registered actions are the following:",
	"",
	"/create <#>",
	"/append <#> <obj>",
	"/insert <#> <idx> <obj>",
	"/remove <#> <obj>",
	"/pop <#> <idx> <expected-obj>",
	"/clear <#>",
	"/copy <#-orig> <#-dest>",
	"/count <#> <obj> <expected-count>",
	"/index <#> <obj> <expected-index>",
	"/size <#> <expected-size>",
	"/iter <#> [<obj1> [<obj2> ...]]",
	"/at <#> <idx> <expected-obj>",
	0,
};

static yadsl_ListHandle* lists[10];

static yadsl_TesterRet global_ret;
static size_t yadsl_list_iter_test_index;

bool yadsl_tester_object_copy_aux(yadsl_ListObj* obj, yadsl_ListObj** copy_ptr)
{
	yadsl_ListObj* copy = yadsl_tester_object_copy(obj);
	if (copy)
		*copy_ptr = copy;
	return copy != NULL;
}

void yadsl_list_iter_test(yadsl_ListObj* obtained)
{
	void* expected = yadsl_tester_object_parse();
	if (expected) {
		if (!yadsl_tester_object_equal(obtained, expected)) {
			yadsl_tester_log("Objects in index %zu differ",
				yadsl_list_iter_test_index);
			global_ret = YADSL_TESTER_RET_RETURN;
		}
		yadsl_tester_object_free(expected);
	} else {
		yadsl_tester_log("Could not allocate expected object at index %zu",
			yadsl_list_iter_test_index);
		global_ret = YADSL_TESTER_RET_MALLOC;
	}
	++yadsl_list_iter_test_index;
}

yadsl_TesterRet yadsl_tester_init()
{
	return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_convert_ret(yadsl_ListRet ret)
{
	switch (ret) {
	case YADSL_LIST_RET_OK:
		return YADSL_TESTER_RET_OK;
	case YADSL_LIST_RET_MEMORY:
		return YADSL_TESTER_RET_MALLOC;
	case YADSL_LIST_RET_INDEX:
		return yadsl_tester_error("index");
	case YADSL_LIST_RET_NOT_FOUND:
		return yadsl_tester_error("not found");
	default:
		return yadsl_tester_error("unknown");
	}
}

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
	yadsl_ListRet ret = YADSL_LIST_RET_OK;
	if (yadsl_testerutils_match(command, "create")) {
		size_t slot;
		yadsl_ListHandle* temp;
		if (yadsl_tester_parse_arguments("z", &slot) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		check_index(slot, lists);
		temp = yadsl_list_create();
		if (temp == NULL)
			return YADSL_TESTER_RET_MALLOC;
		if (lists[slot])
			yadsl_list_destroy(lists[slot], yadsl_tester_object_free);
		lists[slot] = temp;
	} else if (yadsl_testerutils_match(command, "append")) {
		size_t slot;
		void* obj;
		if (yadsl_tester_parse_arguments("z", &slot) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		check_index(slot, lists);
		check_not_null(slot, lists);
		obj = yadsl_tester_object_parse();
		if (obj == NULL)
			return YADSL_TESTER_RET_MALLOC;
		ret = yadsl_list_append(lists[slot], obj);
		if (ret)
			yadsl_tester_object_free(obj);
	} else if (yadsl_testerutils_match(command, "insert")) {
		size_t slot;
		int index;
		void* obj;
		if (yadsl_tester_parse_arguments("zi", &slot, &index) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		check_index(slot, lists);
		check_not_null(slot, lists);
		obj = yadsl_tester_object_parse();
		if (obj == NULL)
			return YADSL_TESTER_RET_MALLOC;
		ret = yadsl_list_insert(lists[slot], (intptr_t) index, obj);
		if (ret)
			yadsl_tester_object_free(obj);
	} else if (yadsl_testerutils_match(command, "remove")) {
		size_t slot;
		void* obj;
		if (yadsl_tester_parse_arguments("z", &slot) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		check_index(slot, lists);
		check_not_null(slot, lists);
		obj = yadsl_tester_object_parse();
		if (obj == NULL)
			return YADSL_TESTER_RET_MALLOC;
		ret = yadsl_list_remove(lists[slot], obj,
			yadsl_tester_object_equal,
			yadsl_tester_object_free);
		yadsl_tester_object_free(obj);
	} else if (yadsl_testerutils_match(command, "pop")) {
		size_t slot;
		int index;
		void* expected, *obtained = NULL;
		if (yadsl_tester_parse_arguments("zi", &slot, &index) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		check_index(slot, lists);
		check_not_null(slot, lists);
		expected = yadsl_tester_object_parse();
		if (expected == NULL)
			return YADSL_TESTER_RET_MALLOC;
		ret = yadsl_list_pop(lists[slot], (intptr_t) index, &obtained);
		if (!ret) {
			bool equal = yadsl_tester_object_equal(expected, obtained);
			yadsl_tester_object_free(obtained);
			if (!equal)
				return YADSL_TESTER_RET_RETURN;
		}
		yadsl_tester_object_free(expected);
	} else if (yadsl_testerutils_match(command, "clear")) {
		size_t slot;
		if (yadsl_tester_parse_arguments("z", &slot) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		check_index(slot, lists);
		check_not_null(slot, lists);
		yadsl_list_clear(lists[slot], yadsl_tester_object_free);
	} else if (yadsl_testerutils_match(command, "copy")) {
		size_t src, dest;
		yadsl_ListHandle* temp;
		if (yadsl_tester_parse_arguments("zz", &src, &dest) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		check_index(src, lists);
		check_index(dest, lists);
		check_not_null(src, lists);
		temp = yadsl_list_copy(lists[src],
			yadsl_tester_object_copy_aux, yadsl_tester_object_free);
		if (temp == NULL)
			return YADSL_TESTER_RET_MALLOC;
		if (lists[dest])
			yadsl_list_destroy(lists[dest], yadsl_tester_object_free);
		lists[dest] = temp;
	} else if (yadsl_testerutils_match(command, "count")) {
		size_t slot, expected, obtained;
		void* obj;
		if (yadsl_tester_parse_arguments("z", &slot) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		check_index(slot, lists);
		check_not_null(slot, lists);
		obj = yadsl_tester_object_parse();
		if (yadsl_tester_parse_arguments("z", &expected) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if (obj == NULL)
			return YADSL_TESTER_RET_MALLOC;
		obtained = yadsl_list_count(lists[slot], obj, yadsl_tester_object_equal);
		yadsl_tester_object_free(obj);
		if (obtained != expected)
			return YADSL_TESTER_RET_RETURN;
	} else if (yadsl_testerutils_match(command, "index")) {
		size_t slot, expected, obtained = -1;
		void* obj;
		if (yadsl_tester_parse_arguments("z", &slot) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		check_index(slot, lists);
		check_not_null(slot, lists);
		obj = yadsl_tester_object_parse();
		if (yadsl_tester_parse_arguments("z", &expected) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if (obj == NULL)
			return YADSL_TESTER_RET_MALLOC;
		ret = yadsl_list_index(lists[slot], obj, yadsl_tester_object_equal, &obtained);
		yadsl_tester_object_free(obj);
		if (!ret && obtained != expected)
			return YADSL_TESTER_RET_RETURN;
	} else if (yadsl_testerutils_match(command, "size")) {
		size_t slot, expected, obtained;
		if (yadsl_tester_parse_arguments("zz", &slot, &expected) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		check_index(slot, lists);
		check_not_null(slot, lists);
		obtained = yadsl_list_size(lists[slot]);
		if (obtained != expected)
			return YADSL_TESTER_RET_RETURN;
	} else if (yadsl_testerutils_match(command, "iter")) {
		size_t slot;
		if (yadsl_tester_parse_arguments("z", &slot) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		check_index(slot, lists);
		check_not_null(slot, lists);
		global_ret = YADSL_TESTER_RET_OK;
		yadsl_list_iter_test_index = 0;
		yadsl_list_iter(lists[slot], yadsl_list_iter_test);
		if (global_ret)
			return global_ret;
	} else if (yadsl_testerutils_match(command, "at")) {
		size_t slot;
		int index;
		void* expected, *obtained;
		if (yadsl_tester_parse_arguments("zi", &slot, &index) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		check_index(slot, lists);
		check_not_null(slot, lists);
		expected = yadsl_tester_object_parse();
		if (expected == NULL)
			return YADSL_TESTER_RET_MALLOC;
		ret = yadsl_list_at(lists[slot], (intptr_t) index, &obtained);
		if (!ret) {
			bool equal = yadsl_tester_object_equal(expected, obtained);
			yadsl_tester_object_free(expected);
			if (!equal)
				return YADSL_TESTER_RET_RETURN;
		} else {
			yadsl_tester_object_free(expected);
		}
	} else {
		return YADSL_TESTER_RET_COUNT;
	}
	return yadsl_tester_convert_ret(ret);
}

yadsl_TesterRet yadsl_tester_release()
{
	for (int i = 0; i < sizeof(lists)/sizeof(*lists); ++i)
		if (lists[i])
			yadsl_list_destroy(lists[i], yadsl_tester_object_free);
	return YADSL_TESTER_RET_OK;
}
