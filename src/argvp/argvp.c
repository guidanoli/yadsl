#include <argvp/argvp.h>

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include <yadsl/stdlib.h>

typedef enum
{
	YADSL_ARGV_TYPE_POSITIONAL = 0,
	YADSL_ARGV_TYPE_KEYWORD,
	YADSL_ARGV_TYPE_KEYWORD_VALUE,
}
yadsl_ArgvType;

typedef struct
{
	int argc;
	char** argv;
	yadsl_ArgvType* argv_types; /* Map from argument vector to argument type */
	int cursor; /* Cursor for iterating through keyword argument values */
}
yadsl_ArgvParser;

#define cast_(argvp) yadsl_ArgvParser* argvp ## _ = (yadsl_ArgvParser *) argvp;

yadsl_ArgvParserHandle*
yadsl_argvp_create(
	int argc,
	char** argv)
{
	yadsl_ArgvParser* argvp = malloc(sizeof *argvp);
	if (argvp) {
		argvp->argc = argc;
		argvp->argv = argv;
		argvp->cursor = 0;
		argvp->argv_types = calloc(argc, sizeof(yadsl_ArgvType));
		if (argvp->argv_types == NULL)
			goto fail;
	}
	return argvp;
fail:
	free(argvp);
	return NULL;
}

void
yadsl_argvp_add_keyword_arguments(
	yadsl_ArgvParserHandle* argvp,
	const yadsl_ArgvKeywordArgumentDef kwargdefs[])
{
	for (; kwargdefs->kw; ++kwargdefs)
		yadsl_argvp_add_keyword_argument(argvp, kwargdefs->kw, kwargdefs->valc);
}

void
yadsl_argvp_add_keyword_argument(
	yadsl_ArgvParserHandle* argvp,
	const char* kw,
	int valc)
{
	bool found;
	cast_(argvp);
	for (int i = 0; i < argvp_->argc - valc; ++i) {

		if (argvp_->argv_types[i])
			continue;

		if (strcmp(kw, argvp_->argv[i]))
			continue;

		found = true;
		for (int j = 1; j <= valc; ++j) {
			if (argvp_->argv_types[i + j]) {
				found = false;
				break;
			}
		}
		if (!found)
			continue;

		argvp_->argv_types[i] = YADSL_ARGV_TYPE_KEYWORD;
		for (int j = 1; j <= valc; ++j)
			argvp_->argv_types[i + j] = YADSL_ARGV_TYPE_KEYWORD_VALUE;
	}
}

int
yadsl_argvp_get_positional_argument_count(
	yadsl_ArgvParserHandle* argvp)
{
	int count = 0;
	cast_(argvp);
	for (int i = 0; i < argvp_->argc; ++i)
		if (!argvp_->argv_types[i])
			++count;
	return count;
}

const char*
yadsl_argvp_get_positional_argument(
	yadsl_ArgvParserHandle* argvp,
	int argi)
{
	int index = 0;
	cast_(argvp);
	for (int i = 0; i < argvp_->argc; ++i)
		if (!argvp_->argv_types[i]) {
			if (index == argi)
				return argvp_->argv[i];
			++index;
		}
	return NULL;
}

int
yadsl_argvp_parse_positional_argument(
	yadsl_ArgvParserHandle* argvp,
	int argi,
	const char* fmt,
	...)
{
	const char* arg = yadsl_argvp_get_positional_argument(argvp, argi);
	if (arg) {
		va_list va;
		int ret;
		va_start(va, fmt);
		ret = vsscanf(arg, fmt, va);
		va_end(va);
		return ret;
	} else {
		return -1;
	}
}

const char*
yadsl_argvp_get_keyword_argument_value(
	yadsl_ArgvParserHandle* argvp,
	const char* kw,
	int vali)
{
	bool found;
	cast_(argvp);

	if (kw) {
		argvp_->cursor = 0;
	} else {
		if (argvp_->cursor <= 0 || argvp_->cursor >= argvp_->argc)
			return NULL;
		kw = argvp_->argv[argvp_->cursor - 1];
	}

	for (int i = argvp_->cursor; i < argvp_->argc - vali - 1; ++i) {

		if (argvp_->argv_types[i] != YADSL_ARGV_TYPE_KEYWORD)
			continue;

		if (strcmp(kw, argvp_->argv[i]))
			continue;

		found = true;
		for (int j = 0; j <= vali; ++j) {
			if (argvp_->argv_types[i + j + 1] != YADSL_ARGV_TYPE_KEYWORD_VALUE) {
				found = false;
				break;
			}
		}
		if (!found)
			continue;

		argvp_->cursor = i + 1;
		return argvp_->argv[i + vali + 1];
	}
	return NULL;
}

int
yadsl_argvp_parse_keyword_argument_value(
	yadsl_ArgvParserHandle* argvp,
	const char* kw,
	int vali,
	const char* fmt,
	...)
{
	const char* arg = yadsl_argvp_get_keyword_argument_value(argvp, kw, vali);
	if (arg) {
		va_list va;
		int ret;
		va_start(va, fmt);
		ret = vsscanf(arg, fmt, va);
		va_end(va);
		return ret;
	} else {
		return -1;
	}
}

int
yadsl_argvp_has_keyword_argument(
    yadsl_ArgvParserHandle* argvp,
    const char* kw)
{
	cast_(argvp);
	for (int i = 0; i < argvp_->argc; ++i)
		if (strcmp(argvp_->argv[i], kw) == 0 &&
			argvp_->argv_types[i] == YADSL_ARGV_TYPE_KEYWORD) {
			return 1;
		}
	return 0;
}

void
yadsl_argvp_destroy(
	yadsl_ArgvParserHandle* argvp)
{
	cast_(argvp);
	free(argvp_->argv_types);
	free(argvp_);
}
