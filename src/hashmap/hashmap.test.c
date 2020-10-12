#include <hashmap/hashmap.h>

#include <stdlib.h>

#include <tester/tester.h>
#include <testerutils/testerutils.h>

const char *yadsl_tester_help_strings[] = {
	"This is the hashmap test module",
	NULL,
};

yadsl_HashMapHandle* map;
const char key[BUFSIZ];

void delfunc(const char* key, void* val) {
	free(val);
}

yadsl_TesterRet convertRet(yadsl_HashMapRet ret)
{
	switch (ret) {
	case YADSL_HASHMAP_RET_OK:
		return YADSL_TESTER_RET_OK;
	case YADSL_HASHMAP_RET_EXISTS:
		return yadsl_tester_return_external_value("exists");
	case YADSL_HASHMAP_RET_DOESNT_EXIST:
		return yadsl_tester_return_external_value("doesnt exist");
	case YADSL_HASHMAP_RET_MEMORY:
		return YADSL_TESTER_RET_MALLOC;
	default:
		return yadsl_tester_return_external_value("unknown");
	}
}

yadsl_TesterRet yadsl_tester_init()
{
	map = yadsl_hashmap_create(4, delfunc);
	if (map == NULL)
		return YADSL_TESTER_RET_MALLOC;
	else
		return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
	yadsl_HashMapRet ret = YADSL_HASHMAP_RET_OK;
	if (yadsl_testerutils_match(command, "new")) {
		int exp;
		if (yadsl_tester_parse_arguments("i", &exp) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		yadsl_hashmap_destroy(map);
		map = yadsl_hashmap_create(4, delfunc);
		if (map == NULL)
			return YADSL_TESTER_RET_MALLOC;
	} else if (yadsl_testerutils_match(command, "add")) {
		int val;
		if (yadsl_tester_parse_arguments("si", key, &val) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		int* val_ptr = malloc(sizeof(int));
		if (!val_ptr)
			return YADSL_TESTER_RET_MALLOC;
		*val_ptr = val;
		if (ret = yadsl_hashmap_entry_add(map, key, val_ptr))
			free(val_ptr);
	} else if (yadsl_testerutils_match(command, "rmv")) {
		if (yadsl_tester_parse_arguments("s", key) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		ret = yadsl_hashmap_entry_remove(map, key);
	} else if (yadsl_testerutils_match(command, "get")) {
		int exp;
		if (yadsl_tester_parse_arguments("si", key, &exp) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		int* obt_ptr;
		ret = yadsl_hashmap_entry_value_get(map, key, &obt_ptr);
		if (ret == YADSL_HASHMAP_RET_OK) {
			if (*obt_ptr != exp)
				return YADSL_TESTER_RET_RETURN;
		}
	} else if (yadsl_testerutils_match(command, "print")) {
		yadsl_hashmap_print(map);
	} else {
		return YADSL_TESTER_RET_COUNT;
	}
	return convertRet(ret);
}

yadsl_TesterRet yadsl_tester_release()
{
	if (map)
		yadsl_hashmap_destroy(map);
	return YADSL_TESTER_RET_OK;
}

