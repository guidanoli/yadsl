#include "map.h"

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tester.h"
#include "testerutils.h"

const char *TesterHelpStrings[] = {
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

TesterReturnValue convertReturn(MapRet mapId)
{
	switch (mapId) {
	case MAP_OK:
		return TESTER_OK;
	case MAP_ENTRY_NOT_FOUND:
		return TesterExternalReturnValue("noentry");
	case MAP_MEMORY:
		return TesterExternalReturnValue("malloc");
	default:
		return TesterExternalReturnValue("unknown");
	}
}

static Map *pMap;
static char key[BUFSIZ], value[BUFSIZ], yn[BUFSIZ];

TesterReturnValue TesterInitCallback()
{
	MapRet mapId;
	if (mapId = mapCreate(&pMap, cmpKeys, freeEntry, NULL))
		return convertReturn(mapId);
	return TESTER_OK;
}

TesterReturnValue TesterParseCallback(const char *command)
{
	MapRet mapId;
	if matches(command, "put") {
		char *temp, *keyStr, *valStr;
		int actual, expected;
		if (TesterParseArguments("sss", key, value, yn) != 3)
			return TESTER_ARGUMENT;
		if ((keyStr = strdup(key)) == NULL)
			return TESTER_MALLOC;
		if ((valStr = strdup(value)) == NULL) {
			free(keyStr);
			return TESTER_MALLOC;
		}
		expected = TesterGetYesOrNoFromString(yn);
		mapId = mapPutEntry(pMap, keyStr, valStr, &actual, &temp);
		if (actual) {
			free(keyStr);
			free(temp);
		} else if (mapId != MAP_OK) {
			free(keyStr);
			free(valStr);
		}
		if (!mapId && actual != expected)
			return TESTER_RETURN;
	} else if matches(command, "get") {
		char *temp, *keyStr;
		if (TesterParseArguments("ss", key, value) != 2)
			return TESTER_ARGUMENT;
		if ((keyStr = strdup(key)) == NULL)
			return TESTER_MALLOC;
		mapId = mapGetEntry(pMap, keyStr, &temp);
		free(keyStr);
		if (mapId == MAP_OK) {
			if (temp == NULL)
				TesterLog("Value returned by mapGetEntry is NULL");
			if (nmatches(value, temp))
				return TESTER_RETURN;
		}
	} else if matches(command, "remove") {
		char *temp, *keyStr, *valStr;
		if (TesterParseArguments("s", key) != 1)
			return TESTER_ARGUMENT;
		if ((temp = strdup(key)) == NULL)
			return TESTER_MALLOC;
		mapId = mapRemoveEntry(pMap, temp, &keyStr, &valStr);
		free(temp);
		if (mapId == MAP_OK) {
			free(keyStr);
			free(valStr);
		}
	} else if matches(command, "nentries") {
		size_t expected, actual;
		if (TesterParseArguments("z", &expected) != 1)
			return TESTER_ARGUMENT;
		mapId = mapGetNumberOfEntries(pMap, &actual);
		if (mapId == MAP_OK && actual != expected)
			return TESTER_RETURN;
	} else {
		return TESTER_COMMAND;
	}
	return convertReturn(mapId);
}

TesterReturnValue TesterExitCallback()
{
	mapDestroy(pMap);
	return TESTER_OK;
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
