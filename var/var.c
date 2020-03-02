#include "var.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#ifdef _DEBUG
static int refCount = 0;
#endif

#pragma once
#if defined(_MSC_VER)
# pragma warning(disable : 4996)
# pragma warning(disable : 4028)
# pragma warning(disable : 6386)
#endif

struct VariableType
{
	const char *typeId;
	int (*compare)(void *var1, void *var2); // 0 = not equal, else, equal
	int (*write)(FILE *fp, void *var); // 0 = success, else, failure
	int (*swrite)(FILE *fp, void *var); // 0 = success, else, failure
	int (*read)(const char *text, void **pVar); // 0 = success, else, failure
};

struct Variable
{
	void *value;
	struct VariableType *type;
};

/* Default variable types */

#define _INT long long
#define _DEC double
#define _STR char *

static int _cmpInteger(_INT *a, _INT *b);
static int _cmpDecimal(_DEC *a, _DEC *b);
static int _cmpString(_STR a, _STR b);

static int _printInteger(FILE *fp, _INT *a);
static int _printDecimal(FILE *fp, _DEC *a);
static int _printString(FILE *fp, _STR a);

static int _swriteInteger(FILE *fp, _INT *a);
static int _swriteDecimal(FILE *fp, _DEC *a);
static int _swriteString(FILE *fp, _STR a);

static int _readInteger(const char *text, _INT **b);
static int _readDecimal(const char *text, _DEC **b);
static int _readString(const char *text, _STR *b);

#define VARTYPE(label) { #label, _cmp ## label, _print ## label, \
	_swrite ## label, _read ## label }

static struct VariableType varTypes[] = {
	VARTYPE(Integer),
	VARTYPE(Decimal),
	VARTYPE(String),
	{ NULL, NULL, NULL, NULL }
};

/* Public functions */

VarReturnID varCreate(const char *text, Variable **ppVariable)
{
	void *value;
	struct Variable *pVariable;
	struct VariableType *pVarType;
	if (text == NULL || ppVariable == NULL)
		return VAR_RETURN_INVALID_PARAMETER;
	pVariable = malloc(sizeof(struct Variable));
	if (pVariable == NULL)
		return VAR_RETURN_MEMORY;
	for (pVarType = varTypes; pVarType->typeId; ++pVarType) {
		if (!pVarType->read(text, &value)) {
			pVariable->value = value;
			pVariable->type = pVarType;
			*ppVariable = pVariable;
#ifdef _DEBUG
			++refCount;
#ifdef _MEMLEAK
			printf("Allocated ");
			varWrite(pVariable, stdout);
			printf("\n");
#endif /* _MEMLEAK */
#endif /* _DEBUG */
			return VAR_RETURN_OK;
		}
	}
	free(pVariable);
	/* Since the last option is always to just duplicate
	* the input text, the only way for the creation of
	* the variable to fail is if strdup fails, which may
	* most likely lay back on memory */
	return VAR_RETURN_MEMORY;
}

static VarReturnID varCreateMultipleRecursive(va_list va)
{
	VarReturnID id;
	const char *text;
	Variable **ppVariable;
	if ((text = va_arg(va, const char *)) == NULL)
		return VAR_RETURN_OK;
	if ((ppVariable = va_arg(va, Variable **)) == NULL)
		return VAR_RETURN_INVALID_PARAMETER;
	if (id = varCreate(text, ppVariable))
		return id;
	id = varCreateMultipleRecursive(va);
	if (id) varDestroy(*ppVariable);
	return id;
}

VarReturnID varCreateMultiple(const char *text, Variable **ppVariable, ...)
{
	va_list va;
	VarReturnID id;
	if (id = varCreate(text, ppVariable))
		return id;
	va_start(va, ppVariable);
	id = varCreateMultipleRecursive(va);
	va_end(va);
	if (id) varDestroy(*ppVariable);
	return id;
}

VarReturnID varCompare(Variable *pVariableA, Variable *pVariableB,
	int *pResult)
{
	if (pVariableA == NULL || pVariableB == NULL || pResult == NULL)
		return VAR_RETURN_INVALID_PARAMETER;
	if (pVariableA == pVariableB)
		*pResult = 1;
	else if (strcmp(pVariableA->type->typeId, pVariableB->type->typeId))
		*pResult = 0;
	else
		*pResult = pVariableA->type->compare(pVariableA->value,
			pVariableB->value);
	return VAR_RETURN_OK;
}

VarReturnID varWrite(Variable *pVariable, FILE *fp)
{
	if (pVariable == NULL)
		return VAR_RETURN_INVALID_PARAMETER;
	if (pVariable->type->write(fp, pVariable->value))
		return VAR_RETURN_WRITING_ERROR;
#ifdef _DEBUG
	printf(" (\"%s\" @ %p)", pVariable->type->typeId, pVariable);
#endif
	return VAR_RETURN_OK;
}

VarReturnID varSerialize(Variable *pVariable, FILE *fp)
{
	if (pVariable == NULL)
		return VAR_RETURN_INVALID_PARAMETER;
	if (pVariable->type->swrite(fp, pVariable->value))
		return VAR_RETURN_WRITING_ERROR;
	return VAR_RETURN_OK;
}

VarReturnID varDeserialize(Variable **ppVariable, FILE *fp)
{
	size_t index = 0, bufferSize = 1024;
	char *buffer, *temp, c, escape = 0;
	VarReturnID ret;
	if (ppVariable == NULL)
		return VAR_RETURN_INVALID_PARAMETER;
	if ((c = fgetc(fp)) != '"')
		return VAR_RETURN_FILE_FORMAT_ERROR;
	buffer = malloc(bufferSize);
	if (buffer == NULL)
		return VAR_RETURN_MEMORY;
	while ((c = fgetc(fp)) != '"' || escape) {
		if (c == EOF) {
			free(buffer);
			return VAR_RETURN_FILE_FORMAT_ERROR;
		}
		if (c == '\\' && !escape) {
			escape = 1;
		} else {
			escape = 0;
			buffer[index] = c;
			++index;
			if (index >= bufferSize) {
				temp = realloc(buffer, bufferSize * 2);
				if (temp == NULL) {
					free(buffer);
					return VAR_RETURN_MEMORY;
				}
				bufferSize *= 2;
				buffer = temp;
			}
		}
	}
	buffer[index] = '\0';
	ret = varCreate(buffer, ppVariable);
	free(buffer);
	return ret;
}


#ifdef _DEBUG
int varGetRefCount()
{
	return refCount;
}
#endif

void varDestroy(Variable *pVariable)
{
	if (pVariable == NULL)
		return;
#ifdef _DEBUG
	--refCount;
#ifdef _MEMLEAK
	printf("Deallocated ");
	varWrite(pVariable, stdout);
	printf("\n");
#endif /* _MEMLEAK */
#endif /* _DEBUG */
	free(pVariable->value);
	free(pVariable);
}

/* Private functions definitions */

// Compare

static int _cmpInteger(_INT *a, _INT *b)
{
	return *a == *b;
}

static int _cmpDecimal(_DEC *a, _DEC *b)
{
	return *a == *b;
}

static int _cmpString(_STR a, _STR b)
{
	return strcmp(a, b) == 0;
}

// Print

static int _printInteger(FILE *fp, _INT *a)
{
	if (a == NULL) return 1;
	if (fprintf(fp, "%lld", *a) < 0) return 1;
	return 0;
}

static int _printDecimal(FILE *fp, _DEC *a)
{
	if (a == NULL) return 1;
	if (fprintf(fp, "%lf", *a) < 0) return 1;
	return 0;
}

static int _printString(FILE *fp, _STR a)
{
	if (a == NULL) return 1;
	if (fprintf(fp, "%s", a) < 0) return 1;
	return 0;
}

// Serialize write

static int _swriteInteger(FILE *fp, _INT *a)
{
	int ret;
	if (fprintf(fp, "\"") < 0) return 1;
	ret = _printInteger(fp, a);
	if (fprintf(fp, "\"") < 0) return 1;
	return ret;
}

static int _swriteDecimal(FILE *fp, _DEC *a)
{
	int ret;
	if (fprintf(fp, "\"") < 0) return 1;
	ret = _printDecimal(fp, a);
	if (fprintf(fp, "\"") < 0) return 1;
	return ret;
}

static int _swriteString(FILE *fp, _STR a)
{
	size_t i, j = 0, strSize = strlen(a), escapeCount = 0;
	char *b;
	int ret;
	for (i = 0; i < strSize; ++i)
		if (a[i] == '"' || a[i] == '\\')
			++escapeCount;
	b = malloc(sizeof(char) * (escapeCount + strSize + 3));
	if (b == NULL)
		return 1;
	b[0] = '"';
	for (i = 0; i < strSize; ++i) {
		if (a[i] == '"' || a[i] == '\\') {
			b[i + j + 1] = '\\';
			b[i + j + 2] = a[i];
			++j;
		} else {
			b[i + j + 1] = a[i];
		}
	}
	b[strSize + escapeCount + 1] = '"';
	b[strSize + escapeCount + 2] = '\0';
	ret = _printString(fp, b);
	free(b);
	return ret;
}

// Read

static int _readInteger(const char *text, _INT **b)
{
	_INT *a;
	char end = 0;
	a = malloc(sizeof(_INT));
	if (a == NULL) return 1;
	if (sscanf(text, "%lld%c", a, &end) != 1 || end) {
		free(a);
		return 1;
	}
	*b = a;
	return 0;
}

static int _readDecimal(const char *text, _DEC **b)
{
	_DEC *a;
	char end = 0;
	a = malloc(sizeof(_DEC));
	if (a == NULL) return 1;
	if (sscanf(text, "%lf%c", a, &end) != 1 || end) {
		free(a);
		return 1;
	}
	*b = a;
	return 0;
}

static int _readString(const char *text, _STR *b)
{
	_STR a;
	a = strdup(text);
	if (a == NULL) return 1;
	*b = a;
	return 0;
}