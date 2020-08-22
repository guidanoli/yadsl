#include <testerutils/testerutils.h>

#include <stdio.h>
#include <stdlib.h>

#include <tester/tester.h>

const char *TesterHelpStrings[] = {
	"This is the testerutils test module",
	"/serialize <string> <file>",
	"/deserialize <string> <file>",
	"/yes [YES/NO]",
	"/no [YES/NO]",
	NULL,
};

TesterReturnValue TesterInitCallback()
{
	return TESTER_OK;
}

char buffer1[BUFSIZ], buffer2[BUFSIZ];

TesterReturnValue TesterParseCallback(const char *command)
{
	if matches(command, "serialize") {
		FILE *file;
		int ret;
		if (TesterParseArguments("ss", buffer1, buffer2) != 2)
			return TESTER_ARGUMENT;
		if ((file = fopen(buffer2, "w")) == NULL)
			return TESTER_FILE;
		ret = TesterUtilsSerializeString(file, buffer1);
		fclose(file);
		if (ret)
			return TesterExternalReturnValue("serialization error");
	} else if matches(command, "deserialize") {
		FILE *file;
		char *string;
		int matches;
		if (TesterParseArguments("ss", buffer1, buffer2) != 2)
			return TESTER_ARGUMENT;
		if ((file = fopen(buffer2, "r")) == NULL)
			return TESTER_FILE;
		string = TesterUtilsDeserializeString(file);
		fclose(file);
		if (string == NULL)
			return TesterExternalReturnValue("deserialization error");
		matches = matches(buffer1, string);
		free(string);
		if (!matches)
			return TESTER_RETURN;
	} else if matches(command, "yes") {
		int yes;
		if (TesterParseArguments("s", buffer1) != 1)
			return TESTER_ARGUMENT;
		yes = TesterGetYesOrNoFromString(buffer1);
		if (!yes)
			return TESTER_RETURN;
	} else if matches(command, "no") {
		int yes;
		if (TesterParseArguments("s", buffer1) != 1)
			return TESTER_ARGUMENT;
		yes = TesterGetYesOrNoFromString(buffer1);
		if (yes)
			return TESTER_RETURN;
	} else {
		return TESTER_COMMAND;
	}
	return TESTER_OK;
}

TesterReturnValue TesterExitCallback()
{
	return TESTER_OK;
}

