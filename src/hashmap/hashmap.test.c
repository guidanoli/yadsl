#include "hashmap.h"

#include "tester.h"
#include "testerutils.h"

const char *TesterHelpStrings[] = {
	"This is the hashmap test module",
	NULL,
};

HashMap* map;
const char key[BUFSIZ];

void delfunc(const char* key, void* val) {}

TesterReturnValue convertRet(HashMapRet ret)
{
	switch (ret) {
	case HASHMAP_OK:
		return TESTER_OK;
	case HASHMAP_EXISTS:
		return TesterExternalReturnValue("exists");
	case HASHMAP_DOESNT_EXIST:
		return TesterExternalReturnValue("doesnt exist");
	case HASHMAP_MEMORY:
		return TesterExternalReturnValue("memory");
	default:
		return TesterExternalReturnValue("unknown");
	}
}

TesterReturnValue TesterInitCallback()
{
	map = NULL;
	HashMapRet ret = hashMapCreate(&map, 4, delfunc);
	return convertRet(ret);
}

TesterReturnValue TesterParseCallback(const char *command)
{
	HashMapRet ret = HASHMAP_OK;
	if matches(command, "new") {
		int exp;
		if (TesterParseArguments("i", &exp) != 1)
			return TESTER_ARGUMENT;
		hashMapDestroy(map);
		map = NULL;
		ret = hashMapCreate(&map, 4, delfunc);
	} else if matches(command, "add") {
		int val;
		if (TesterParseArguments("si", key, &val) != 2)
			return TESTER_ARGUMENT;
		ret = hashMapAddEntry(map, key, (void*) val);
	} else if matches(command, "rmv") {
		if (TesterParseArguments("s", key) != 1)
			return TESTER_ARGUMENT;
		ret = hashMapRemoveEntry(map, key);
	} else if matches(command, "get") {
		int exp;
		if (TesterParseArguments("si", key, &exp) != 2)
			return TESTER_ARGUMENT;
		void* obt;
		ret = hashMapGetEntry(map, key, &obt);
		if (ret == HASHMAP_OK) {
			if ((int) obt != exp)
				return TESTER_RETURN;
		}
	} else if matches(command, "print") {
		hashMapPrint(map);
	} else {
		return TESTER_COUNT;
	}
	return convertRet(ret);
}

TesterReturnValue TesterExitCallback()
{
	if (map)
		hashMapDestroy(map);
	return TESTER_OK;
}

