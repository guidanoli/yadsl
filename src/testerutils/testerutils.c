#include <testerutils/testerutils.h>

#include <string.h>

#include <string/string.h>
#include <tester/tester.h>

#if defined(_MSC_VER)
# pragma warning(disable : 4996)
#endif

bool
yadsl_testerutils_match(
	const char* a,
	const char* b)
{
	return yadsl_string_compare_ic(a, b) == 0;
}

bool
yadsl_testerutils_str_serialize(
	FILE* fp,
	const char* string)
{
	return fprintf(fp, "%zu~%s", strlen(string), string) < 0;
}

char*
yadsl_testerutils_str_deserialize(
	FILE* fp)
{
	size_t size = 0;
	char* string;
	if (fscanf(fp, "%zu~", &size) != 1)
		goto fail1;
	string = malloc(size + 1);
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

bool
yadsl_testerutils_str_to_bool(
	const char* string)
{
	static const char* yes_words[] = {
		"1",
		"yes",
		"true",
		NULL
	};
	static const char* no_words[] = {
		"0",
		"no",
		"false",
		NULL
	};
	for (const char** yes_word = yes_words; *yes_word; ++yes_word)
		if (yadsl_testerutils_match(string, *yes_word))
			return true;
	for (const char** no_word = no_words; *no_word; ++no_word)
		if (yadsl_testerutils_match(string, *no_word))
			return false;
	yadsl_tester_log("Invalid string \"%s\". Assumed \"no\".", string);
	return false;
}
