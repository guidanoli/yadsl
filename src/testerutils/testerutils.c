#include <testerutils/testerutils.h>

#include <yadsl/posixstring.h>
#include <stdlib.h>

#include <tester/tester.h>

bool
yadsl_testerutils_match(
	const char* a,
	const char* b)
{
	return !strcmp(a, b);
}

bool
yadsl_testerutils_unmatch(
	const char* a,
	const char* b)
{
	return strcmp(a, b);
}

bool yadsl_testerutils_str_serialize(FILE* fp, const char* string)
{
	return fprintf(fp, "%zu~%s", strlen(string), string) < 0;
}

char* yadsl_testerutils_str_deserialize(FILE* fp)
{
	size_t size = 0;
	char* string;
	if (fscanf(fp, "%zu~", &size) != 1)
		goto fail1;
	string = malloc(sizeof(char) * (size + 1));
	if (string) {
		size_t i = 0;
		for (; i < size; ++i) {
			int c = fgetc(fp);
			if (c == EOF)
				goto fail2;
			string[i] = (char) c;
		}
		string[size] = '\0';
	}
	return string;
fail2:
	free(string);
fail1:
	return NULL;
}

bool yadsl_testerutils_str_to_bool(const char* string)
{
	bool yes = yadsl_testerutils_match(string, "YES");
	if (!yes && !yadsl_testerutils_match(string, "NO"))
		yadsl_tester_log("Expected YES or NO. Got '%s'. Assumed NO.", string);
	return yes;
}
