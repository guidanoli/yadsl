#include <testerutils/testerutils.h>

#include <stdlib.h>

#include <tester/tester.h>

#if defined(_MSC_VER)
# pragma warning(disable : 4996)
#endif

int TesterUtilsSerializeString(FILE *fp, const char *string)
{
	return fprintf(fp, "%zu~%s", strlen(string), string) < 0;
}

char *TesterUtilsDeserializeString(FILE *fp)
{
	size_t size = 0;
	char *string;
	if (fscanf(fp, "%zu~", &size) != 1)
		return NULL;
	string = malloc(sizeof(char) * (size + 1));
	if (string) {
		size_t i = 0;
		for (; i < size; ++i) {
			int c = fgetc(fp);
			if (c == EOF) {
				free(string);
				return NULL;
			}
			string[i] = c;
		}
		string[size] = '\0';
	}
	return string;
}

bool TesterUtilsGetYesOrNoFromString(const char *string)
{
	bool yes = matches(string, "YES");
	if (!yes && !matches(string, "NO"))
		yadsl_tester_log("Expected YES or NO. Got '%s'. Assumed NO.", string);
	return yes;
}
