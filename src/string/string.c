#include <string/string.h>

#include <string.h>
#include <ctype.h>

#ifdef YADSL_DEBUG
#include <memdb/memdb.h>
#else
#include <stdlib.h>
#endif

#if defined(_MSC_VER)
# pragma warning(disable : 4996)
#endif

char*
yadsl_string_duplicate(
	const char* string)
{
	char* newstring = malloc(strlen(string) + (size_t) 1);
	
	if (newstring)
		strcpy(newstring, string);
	
	return newstring;
}

int
yadsl_string_compare_ic(
	const char* string_a,
	const char* string_b)
{
	char a, b;
	
	while (1) {
		a = tolower(*string_a);
		b = tolower(*string_b);
		
		if (a == '\0' || b == '\0' || a != b)
			break;
		
		++string_a;
		++string_b;
	}
	
	return (int) (a - b);
}