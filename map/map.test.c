#include "map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "tester.h"
#include "var.h"

const char *TesterHelpStrings[] = {
	"This is an interactive module of the map library",
	"You interact with a single map object at all times",
	"",
	"The available commands are:",
	"/put <key> <value>            assign to the key K, the value V",
	"/get <key> <expected value>   obtain the value assigned to key K",
	"/remove <key>                 remove entry of key K",
	"/nentries <expected value>    obtain number of entries",
	NULL, /* Sentinel */
};

// Variable functions
static int cmpVariables(struct Variable *pVariableA,
	struct Variable *pVariableB);
static void freeVariable(struct Variable *pKeyVariable,
	struct Variable *pValueVariable, void *arg);

TesterReturnValue convertReturn(MapReturnID mapId)
{
	switch (mapId) {
	case MAP_RETURN_OK:
		return TESTER_RETURN_OK;
	case MAP_RETURN_OVERWROTE_ENTRY:
		return TesterExternalReturnValue("overwrite");
	case MAP_RETURN_ENTRY_NOT_FOUND:
		return TesterExternalReturnValue("noentry");
	case MAP_RETURN_INVALID_PARAMETER:
		return TesterExternalReturnValue("invalid");
	case MAP_RETURN_MEMORY:
		return TesterExternalReturnValue("malloc");
	default:
		return TesterExternalReturnValue("unknown");
	}
}

static Map *pMap;
static char key[BUFSIZ], value[BUFSIZ];

TesterReturnValue TesterInitCallback()
{
	MapReturnID mapId;
	if (mapId = mapCreate(&pMap, cmpVariables, freeVariable, NULL))
		return convertReturn(mapId);
	return TESTER_RETURN_OK;
}

#define matches(a, b) (strcmp(a, b) == 0)

TesterReturnValue TesterParseCallback(const char *command)
{
	MapReturnID mapId;
	if matches(command, "put") {
		Variable *temp, *pVarKey, *pVarValue;
		if (TesterParseArguments("ss", key, value) != 2)
			return TESTER_RETURN_ARGUMENT;
		if (varCreate(key, &pVarKey))
			return TESTER_RETURN_MALLOC;
		if (varCreate(value, &pVarValue)) {
			varDestroy(pVarKey);
			return TESTER_RETURN_MALLOC;
		}
		mapId = mapPutEntry(pMap, pVarKey, pVarValue, &temp);
		if (mapId == MAP_RETURN_OVERWROTE_ENTRY) {
			varDestroy(temp);
		} else if (mapId != MAP_RETURN_OK) {
			varDestroy(pVarKey);
			varDestroy(pVarValue);
		}
	} else if matches(command, "get") {
		Variable *temp, *pVarKey, *pVarValue;
		if (TesterParseArguments("ss", key, value) != 2)
			return TESTER_RETURN_ARGUMENT;
		if (varCreate(key, &pVarKey))
			return TESTER_RETURN_MALLOC;
		mapId = mapGetEntry(pMap, pVarKey, &temp);
		varDestroy(pVarKey);
		if (mapId == MAP_RETURN_OK) {
			int isEqual;
			if (temp == NULL)
				TesterLog("Value returned by mapGetEntry is NULL");
			if (varCreate(value, &pVarValue))
				return TESTER_RETURN_MALLOC;
			varCompare(temp, pVarValue, &isEqual);
			varDestroy(pVarValue);
			if (!isEqual)
				return TESTER_RETURN_RETURN;
		}
	} else if matches(command, "remove") {
		Variable *temp, *pVarKey, *pVarValue;
		if (TesterParseArguments("s", key) != 1)
			return TESTER_RETURN_ARGUMENT;
		if (varCreate(key, &temp))
			return TESTER_RETURN_MALLOC;
		mapId = mapRemoveEntry(pMap, temp, &pVarKey, &pVarValue);
		varDestroy(temp);
		if (mapId == MAP_RETURN_OK) {
			varDestroy(pVarKey);
			varDestroy(pVarValue);
		}
	} else if matches(command, "nentries") {
		unsigned long expected, actual;
		if (TesterParseArguments("l", &expected) != 1)
			return TESTER_RETURN_ARGUMENT;
		mapId = mapGetNumberOfEntries(pMap, &actual);
		if (mapId == MAP_RETURN_OK && actual != expected)
			return TESTER_RETURN_RETURN;
	} else {
		return TESTER_RETURN_COMMAND;
	}
	return convertReturn(mapId);
}

TesterReturnValue TesterExitCallback()
{
	mapDestroy(pMap);
	return TESTER_RETURN_OK;
}

static void freeVariable(struct Variable *pKeyVariable,
	struct Variable *pValueVariable, void *arg)
{
	varDestroy(pKeyVariable);
	varDestroy(pValueVariable);
}

static int cmpVariables(struct Variable *pVariableA,
	struct Variable *pVariableB)
{
	int cmpReturn = 0; // by default, aren't equal
	if (varCompare(pVariableA, pVariableB, &cmpReturn))
		TesterLog("Error while trying to compare variables");
	return cmpReturn;
}
