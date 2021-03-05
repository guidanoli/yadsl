#include <testerutils/testerutils.h>

#include <string.h>
#include <errno.h>

#include <string/string.h>
#include <tester/tester.h>

#if defined(_MSC_VER)
# pragma warning(disable : 4996)
#endif

struct tempfile
{
	char* filename;
	struct tempfile* next; /* nullable */
};

static struct tempfile* tempfilelist;

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

bool
yadsl_testerutils_add_tempfile_to_list(
	const char* filename)
{
	struct tempfile* temp;
	for (temp = tempfilelist; temp; temp = temp->next)
		if (strcmp(temp->filename, filename) == 0)
			return true;
	temp = malloc(sizeof * temp);
	if (temp == NULL)
		goto fail;
	temp->filename = yadsl_string_duplicate(filename);
	if (temp->filename == NULL)
		goto fail2;
	temp->next = tempfilelist;
	tempfilelist = temp;
	return true;
fail2:
	free(temp);
fail:
	if (remove(filename) != 0) {
		fprintf(stderr, "Could not remove file %s: %s\n",
			filename, strerror(errno));
	}
	return false;
}

void
yadsl_testerutils_clear_tempfile_list()
{
	while (tempfilelist != NULL) {
		struct tempfile* next;
		next = tempfilelist->next;
		if (remove(tempfilelist->filename) != 0) {
			fprintf(stderr, "Could not remove file %s: %s\n",
				tempfilelist->filename, strerror(errno));
		}
		free(tempfilelist->filename);
		free(tempfilelist);
		tempfilelist = next;
	}
}

bool
yadsl_testerutils_compare_file_and_string(
    FILE* fp, const char* string)
{
	int ch;
	do {
		ch = fgetc(fp);
		if (ch == EOF) return *string == '\0';
		else if (*string == '\0') return false;
	} while ((char) ch == *(string++));
	return false;
}
