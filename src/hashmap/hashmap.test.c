#include <hashmap/hashmap.h>

#include <stdlib.h>

#include <tester/tester.h>
#include <testerutils/testerutils.h>

const char *yadsl_tester_help_strings[] = {
	"This is the hashmap test module",
	NULL,
};

HashMap* map;
const char key[BUFSIZ];

void delfunc(const char* key, void* val) {
	free(val);
}

yadsl_TesterRet convertRet(HashMapRet ret)
{
	switch (ret) {
	case HASHMAP_OK:
		return YADSL_TESTER_RET_OK;
	case HASHMAP_EXISTS:
		return yadsl_tester_return_external_value("exists");
	case HASHMAP_DOESNT_EXIST:
		return yadsl_tester_return_external_value("doesnt exist");
	case HASHMAP_MEMORY:
		return yadsl_tester_return_external_value("memory");
	default:
		return yadsl_tester_return_external_value("unknown");
	}
}

yadsl_TesterRet yadsl_tester_init()
{
	map = NULL;
	HashMapRet ret = hashMapCreate(&map, 4, delfunc);
	return convertRet(ret);
}

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
	HashMapRet ret = HASHMAP_OK;
	if matches(command, "new") {
		int exp;
		if (yadsl_tester_parse_arguments("i", &exp) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		hashMapDestroy(map);
		map = NULL;
		ret = hashMapCreate(&map, 4, delfunc);
	} else if matches(command, "add") {
		int val;
		if (yadsl_tester_parse_arguments("si", key, &val) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		int* val_ptr = malloc(sizeof(int));
		if (!val_ptr)
			return YADSL_TESTER_RET_MALLOC;
		*val_ptr = val;
		if (ret = hashMapAddEntry(map, key, val_ptr))
			free(val_ptr);
	} else if matches(command, "rmv") {
		if (yadsl_tester_parse_arguments("s", key) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		ret = hashMapRemoveEntry(map, key);
	} else if matches(command, "get") {
		int exp;
		if (yadsl_tester_parse_arguments("si", key, &exp) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		int* obt_ptr;
		ret = hashMapGetEntry(map, key, &obt_ptr);
		if (ret == HASHMAP_OK) {
			if (*obt_ptr != exp)
				return YADSL_TESTER_RET_RETURN;
		}
	} else if matches(command, "print") {
		hashMapPrint(map);
	} else {
		return YADSL_TESTER_RET_COUNT;
	}
	return convertRet(ret);
}

yadsl_TesterRet yadsl_tester_release()
{
	if (map)
		hashMapDestroy(map);
	return YADSL_TESTER_RET_OK;
}

