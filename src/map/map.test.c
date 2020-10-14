#include <map/map.h>

#include <string.h>
#include <stdio.h>

#include <string/string.h>
#include <tester/tester.h>
#include <testerutils/testerutils.h>

const char *yadsl_tester_help_strings[] = {
	"This is an interactive module of the map library",
	"You interact with a single map object at all times",
	"",
	"The available commands are:",
	"/put <key> <value> [YES/NO]   assign to the key K, the value V",
	"                              test if an entry is overwritten or not",
	"/get <key> <expected value>   obtain the value assigned to key K",
	"/remove <key>                 remove entry of key K",
	"/nentries <expected value>    obtain number of entries",
	NULL, /* Sentinel */
};

static int cmp_keys_func(void *a, void *b, void *arg);
static void free_entry_func(void *k, void *v, void *arg);

yadsl_TesterRet convertReturn(yadsl_MapRet mapId)
{
	switch (mapId) {
	case YADSL_MAP_RET_OK:
		return YADSL_TESTER_RET_OK;
	case YADSL_MAP_RET_ENTRY_NOT_FOUND:
		return yadsl_tester_return_external_value("noentry");
	case YADSL_MAP_RET_MEMORY:
		return YADSL_TESTER_RET_MALLOC;
	default:
		return yadsl_tester_return_external_value("unknown");
	}
}

static yadsl_MapHandle *pMap;
static char key[BUFSIZ], value[BUFSIZ], yn[BUFSIZ];

yadsl_TesterRet yadsl_tester_init()
{
	if (pMap = yadsl_map_create(cmp_keys_func, free_entry_func, NULL, NULL))
		return YADSL_TESTER_RET_OK;
	else
		return YADSL_TESTER_RET_MALLOC;
}

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
	yadsl_MapRet mapId;
	if (yadsl_testerutils_match(command, "put")) {
		char *temp, *keyStr, *valStr;
		bool actual, expected;
		if (yadsl_tester_parse_arguments("sss", key, value, yn) != 3)
			return YADSL_TESTER_RET_ARGUMENT;
		if ((keyStr = yadsl_string_duplicate(key)) == NULL)
			return YADSL_TESTER_RET_MALLOC;
		if ((valStr = yadsl_string_duplicate(value)) == NULL) {
			free(keyStr);
			return YADSL_TESTER_RET_MALLOC;
		}
		expected = yadsl_testerutils_str_to_bool(yn);
		mapId = yadsl_map_entry_add(pMap, keyStr, valStr, &actual, &temp);
		if (actual) {
			free(keyStr);
			free(temp);
		} else if (mapId != YADSL_MAP_RET_OK) {
			free(keyStr);
			free(valStr);
		}
		if (!mapId && actual != expected)
			return YADSL_TESTER_RET_RETURN;
	} else if (yadsl_testerutils_match(command, "get")) {
		char *temp, *keyStr;
		if (yadsl_tester_parse_arguments("ss", key, value) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		if ((keyStr = yadsl_string_duplicate(key)) == NULL)
			return YADSL_TESTER_RET_MALLOC;
		mapId = yadsl_map_entry_get(pMap, keyStr, &temp);
		free(keyStr);
		if (mapId == YADSL_MAP_RET_OK) {
			if (temp == NULL) {
				yadsl_tester_log("Value returned by mapGetEntry is NULL");
				return YADSL_TESTER_RET_RETURN;
			}
			if (strcmp(value, temp))
				return YADSL_TESTER_RET_RETURN;
		}
	} else if (yadsl_testerutils_match(command, "remove")) {
		char *temp, *keyStr, *valStr;
		if (yadsl_tester_parse_arguments("s", key) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if ((temp = yadsl_string_duplicate(key)) == NULL)
			return YADSL_TESTER_RET_MALLOC;
		mapId = yadsl_map_entry_remove(pMap, temp, &keyStr, &valStr);
		free(temp);
		if (mapId == YADSL_MAP_RET_OK) {
			free(keyStr);
			free(valStr);
		}
	} else if (yadsl_testerutils_match(command, "nentries")) {
		size_t expected, actual;
		if (yadsl_tester_parse_arguments("z", &expected) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		mapId = yadsl_map_entry_count_get(pMap, &actual);
		if (mapId == YADSL_MAP_RET_OK && actual != expected)
			return YADSL_TESTER_RET_RETURN;
	} else {
		return YADSL_TESTER_RET_COMMAND;
	}
	return convertReturn(mapId);
}

yadsl_TesterRet yadsl_tester_release()
{
	yadsl_map_destroy(pMap);
	return YADSL_TESTER_RET_OK;
}

static void free_entry_func(void *k, void *v, void *arg)
{
	free(k);
	free(v);
}

static int cmp_keys_func(void *a, void *b, void *arg)
{
	return strcmp((char *) a, (char *) b) == 0;
}
