#include <testerutils/testerutils.h>

#include <stdio.h>
#include <stdlib.h>

#include <tester/tester.h>

const char *yadsl_tester_help_strings[] = {
	"This is the testerutils test module",
	"/serialize <string> <file>",
	"/deserialize <string> <file>",
	"/yes [YES/NO]",
	"/no [YES/NO]",
	NULL,
};

yadsl_TesterRet yadsl_tester_init()
{
	return YADSL_TESTER_RET_OK;
}

char buffer1[BUFSIZ], buffer2[BUFSIZ];

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
	if (yadsl_testerutils_match(command, "serialize")) {
		FILE *file;
		int ret;
		if (yadsl_tester_parse_arguments("ss", buffer1, buffer2) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		if ((file = fopen(buffer2, "w")) == NULL)
			return YADSL_TESTER_RET_FILE;
		ret = yadsl_testerutils_str_serialize(file, buffer1);
		fclose(file);
		if (ret)
			return yadsl_tester_return_external_value("serialization error");
	} else if (yadsl_testerutils_match(command, "deserialize")) {
		FILE *file;
		char *string;
		int matches;
		if (yadsl_tester_parse_arguments("ss", buffer1, buffer2) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		if ((file = fopen(buffer2, "r")) == NULL)
			return YADSL_TESTER_RET_FILE;
		string = yadsl_testerutils_str_deserialize(file);
		fclose(file);
		if (string == NULL)
			return yadsl_tester_return_external_value("deserialization error");
		matches = yadsl_testerutils_match(buffer1, string);
		free(string);
		if (!matches)
			return YADSL_TESTER_RET_RETURN;
	} else if (yadsl_testerutils_match(command, "yes")) {
		int yes;
		if (yadsl_tester_parse_arguments("s", buffer1) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		yes = yadsl_testerutils_str_to_bool(buffer1);
		if (!yes)
			return YADSL_TESTER_RET_RETURN;
	} else if (yadsl_testerutils_match(command, "no")) {
		int yes;
		if (yadsl_tester_parse_arguments("s", buffer1) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		yes = yadsl_testerutils_str_to_bool(buffer1);
		if (yes)
			return YADSL_TESTER_RET_RETURN;
	} else {
		return YADSL_TESTER_RET_COMMAND;
	}
	return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_release()
{
	return YADSL_TESTER_RET_OK;
}

