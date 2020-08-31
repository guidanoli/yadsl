#include <map/map.h>

#include <yadsl/posixstring.h>
#include <stdio.h>
#include <stdlib.h>

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

static int cmpKeys(void *a, void *b);
static void freeEntry(void *k, void *v, void *arg);

yadsl_TesterRet convertReturn(MapRet mapId)
{
	switch (mapId) {
	case MAP_OK:
		return YADSL_TESTER_RET_OK;
	case MAP_ENTRY_NOT_FOUND:
		return yadsl_tester_return_external_value("noentry");
	case MAP_MEMORY:
		return yadsl_tester_return_external_value("malloc");
	default:
		return yadsl_tester_return_external_value("unknown");
	}
}

static Map *pMap;
static char key[BUFSIZ], value[BUFSIZ], yn[BUFSIZ];

yadsl_TesterRet yadsl_tester_init()
{
	MapRet mapId;
	if (mapId = mapCreate(&pMap, cmpKeys, freeEntry, NULL))
		return convertReturn(mapId);
	return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
	MapRet mapId;
	if matches(command, "put") {
		char *temp, *keyStr, *valStr;
		int actual, expected;
		if (yadsl_tester_parse_arguments("sss", key, value, yn) != 3)
			return YADSL_TESTER_RET_ARGUMENT;
		if ((keyStr = strdup(key)) == NULL)
			return YADSL_TESTER_RET_MALLOC;
		if ((valStr = strdup(value)) == NULL) {
			free(keyStr);
			return YADSL_TESTER_RET_MALLOC;
		}
		expected = TesterUtilsGetYesOrNoFromString(yn);
		mapId = mapPutEntry(pMap, keyStr, valStr, &actual, &temp);
		if (actual) {
			free(keyStr);
			free(temp);
		} else if (mapId != MAP_OK) {
			free(keyStr);
			free(valStr);
		}
		if (!mapId && actual != expected)
			return YADSL_TESTER_RET_RETURN;
	} else if matches(command, "get") {
		char *temp, *keyStr;
		if (yadsl_tester_parse_arguments("ss", key, value) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		if ((keyStr = strdup(key)) == NULL)
			return YADSL_TESTER_RET_MALLOC;
		mapId = mapGetEntry(pMap, keyStr, &temp);
		free(keyStr);
		if (mapId == MAP_OK) {
			if (temp == NULL)
				yadsl_tester_log("Value returned by mapGetEntry is NULL");
			if (nmatches(value, temp))
				return YADSL_TESTER_RET_RETURN;
		}
	} else if matches(command, "remove") {
		char *temp, *keyStr, *valStr;
		if (yadsl_tester_parse_arguments("s", key) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if ((temp = strdup(key)) == NULL)
			return YADSL_TESTER_RET_MALLOC;
		mapId = mapRemoveEntry(pMap, temp, &keyStr, &valStr);
		free(temp);
		if (mapId == MAP_OK) {
			free(keyStr);
			free(valStr);
		}
	} else if matches(command, "nentries") {
		size_t expected, actual;
		if (yadsl_tester_parse_arguments("z", &expected) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		mapId = mapGetNumberOfEntries(pMap, &actual);
		if (mapId == MAP_OK && actual != expected)
			return YADSL_TESTER_RET_RETURN;
	} else {
		return YADSL_TESTER_RET_COMMAND;
	}
	return convertReturn(mapId);
}

yadsl_TesterRet yadsl_tester_release()
{
	mapDestroy(pMap);
	return YADSL_TESTER_RET_OK;
}

static void freeEntry(void *k, void *v, void *arg)
{
	free(k);
	free(v);
}

static int cmpKeys(void *a, void *b)
{
	return matches((char *) a, (char *) b);
}
