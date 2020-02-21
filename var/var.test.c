#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "var.h"
#include "tester.h"

#ifndef VARCNT
#define VARCNT 10
#endif

#define _STR(X) #X
#define XSTR(X) _STR(X)

#define matches(a, b) (strcmp(a, b) == 0)
#define isValidIndex(i) (i >= 0 && i < VARCNT)

static Variable *vars[VARCNT];
static char buffer[BUFSIZ], buffer2[BUFSIZ];

static TesterReturnValue convertReturnValue(VarReturnID varId);

TesterReturnValue TesterInitCallback()
{
	static Variable **pVars = vars;
	size_t i, size = sizeof(vars) / sizeof(*vars);
	for (i = 0; i < size; ++i) *(pVars++) = NULL;
	return TESTER_RETURN_OK;
}

TesterReturnValue TesterParseCallback(const char *command)
{
	VarReturnID varId = VAR_RETURN_OK;
	Variable *temp, *temp2;
	int idx1, idx2, expected, obtained;
	FILE *f;
	if matches(command, "add") {
		if (TesterParseArguments("is", &idx1, buffer) != 2)
			return TESTER_RETURN_ARGUMENT;
		if (!isValidIndex(idx1))
			return TesterExternalReturnValue("index");
		temp = vars[idx1];
		varId = varCreate(buffer, &vars[idx1]);
		if (!varId && temp)
			varDestroy(temp);
	} else if matches(command, "add2") {
		if (TesterParseArguments("isis", &idx1, buffer, &idx2, buffer2) != 4)
			return TESTER_RETURN_ARGUMENT;
		if (!isValidIndex(idx1) || !isValidIndex(idx2) || idx1 == idx2)
			return TesterExternalReturnValue("index");
		temp = vars[idx1];
		temp2 = vars[idx2];
		varId = varCreateMultiple(buffer, &vars[idx1], buffer2, &vars[idx2],
			NULL);
		if (!varId) {
			if (temp) varDestroy(temp);
			if (temp2) varDestroy(temp2);
		}
	} else if matches(command, "rmv") {
		if (TesterParseArguments("i", &idx1) != 1)
			return TESTER_RETURN_ARGUMENT;
		if (!isValidIndex(idx1))
			return TesterExternalReturnValue("index");
		if (!vars[idx1])
			return TesterExternalReturnValue("nullvar");
		varDestroy(vars[idx1]);
		vars[idx1] = NULL;
	} else if matches(command, "cmp") {
		if (TesterParseArguments("iis", &idx1, &idx2, buffer) != 3)
			return TESTER_RETURN_ARGUMENT;
		if (!isValidIndex(idx1) || !isValidIndex(idx2))
			return TesterExternalReturnValue("index");
		if (!vars[idx1] || !vars[idx2])
			return TesterExternalReturnValue("nullvar");
		expected = matches(buffer, "EQUAL");
		varId = varCompare(vars[idx1], vars[idx2], &obtained);
		if (varId == VAR_RETURN_OK && obtained != expected)
			return TESTER_RETURN_RETURN;
	} else if matches(command, "dmp") {
		if (TesterParseArguments("i", &idx1) != 1)
			return TESTER_RETURN_ARGUMENT;
		if (!isValidIndex(idx1))
			return TesterExternalReturnValue("index");
		if (!vars[idx1])
			return TesterExternalReturnValue("nullvar");
		varId = varWrite(vars[idx1], stdout);
		if (varId == VAR_RETURN_OK)
			printf("\n");
	} else if matches(command, "dsr") {
		if (TesterParseArguments("is", &idx1, buffer) != 2)
			return TESTER_RETURN_ARGUMENT;
		if (!isValidIndex(idx1))
			return TesterExternalReturnValue("index");
		temp = vars[idx1];
		if ((f = fopen(buffer, "r")) == NULL)
			return TESTER_RETURN_FILE;
		varId = varDeserialize(&vars[idx1], f);
		if (!varId && temp)
			varDestroy(temp);
		fclose(f);
	} else if matches(command, "ser") {
		if (TesterParseArguments("is", &idx1, buffer) != 2)
			return TESTER_RETURN_ARGUMENT;
		if (!isValidIndex(idx1))
			return TesterExternalReturnValue("index");
		if (!vars[idx1])
			return TesterExternalReturnValue("nullvar");
		if ((f = fopen(buffer, "w")) == NULL)
			return TESTER_RETURN_FILE;
		varId = varSerialize(vars[idx1], f);
		fclose(f);
	} else if matches(command, "wef") {
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_RETURN_ARGUMENT;
		if ((f = fopen(buffer, "w")) == NULL)
			return TESTER_RETURN_FILE;
		fclose(f);
	} else {
		return TESTER_RETURN_COMMAND;
	}
	return convertReturnValue(varId);
}

TesterReturnValue TesterExitCallback()
{
	size_t i;
	for (i = 0; i < VARCNT; ++i) {
		if (vars[i] != NULL) {
			varDestroy(vars[i]);
			vars[i] = NULL;
		}
	}
#ifdef _DEBUG
	if (varGetRefCount()) {
		puts("Memory leak detected");
		return TESTER_RETURN_MEMLEAK;
	}
#endif
	return TESTER_RETURN_OK;
}

static TesterReturnValue convertReturnValue(VarReturnID varId)
{
	switch (varId) {
	case VAR_RETURN_OK:
		return TESTER_RETURN_OK;
	case VAR_RETURN_INVALID_PARAMETER:
		return TesterExternalReturnValue("parameter");
	case VAR_RETURN_FILE_FORMAT_ERROR:
		return TesterExternalReturnValue("fileformat");
	case VAR_RETURN_WRITING_ERROR:
		return TesterExternalReturnValue("write");
	case VAR_RETURN_MEMORY:
		return TesterExternalReturnValue("malloc");
	default:
		return TesterExternalReturnValue("unknown");
	}
}

const char *TesterHelpStrings[] = {
	"This is an interactive module of the variable library",
	"You can interact with up to " XSTR(VARCNT_S) " variables at a time",
	"Any command called with an invalid index will throw 'index' error",
	"Any try of access to an unitialized variable will throw 'nullvar' error",
	"The 'malloc' error is not explicit here since it is hardly reproducible",
	"",
	"/add <index> <text>                add variable from text to index",
	"/add2 <idx1> <txt1> <idx2> <txt2>  add two variables from text to index",
	"/rmv <index>                       remove variable at index",
	"/cmp <index1> <index2> [EQUAL|*]   compare variables from two indices",
	"/dmp <index>                       dump variable contents",
	"/ser <index> <filename>            serialize to file",
	"/dsr <index> <filename>            deserialize from file",
	"/wef <filename>                    write empty file",
	NULL,
};
