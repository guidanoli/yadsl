#include <stdlib.h>
#include <string.h>
#include "var.h"

#ifdef _DEBUG
static int refCount = 0;
#endif

struct VariableType {
    const char *typeId;
    int (*compare)(void *var1, void *var2); // 0 = not equal, else, equal
    int (*write)(FILE *fp, void *var); // 0 = success, else, failure
    int (*read)(const char *text, void **pVar); // 0 = success, else, failure
};

struct Variable {
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

static int _readInteger(const char *text, _INT **b);
static int _readDecimal(const char *text, _DEC **b);
static int _readString(const char *text, _STR *b);

#define VARTYPE(label) { #label, _cmp ## label, _print ## label, \
    _read ## label }

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
#endif
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
        *pResult = pVariableA->type->compare(pVariableA->value, pVariableB->value);
    return VAR_RETURN_OK;
}

VarReturnID varWrite(Variable *pVariable, FILE *fp)
{
    if (pVariable == NULL)
        return VAR_RETURN_INVALID_PARAMETER;
    if (pVariable->type->write(fp, pVariable->value))
        return VAR_RETURN_WRITING_ERROR;
#ifdef _DEBUG
    printf(" (\"%s\" @ %#x)", pVariable->type->typeId, pVariable);
#endif
    return VAR_RETURN_OK;
}

#ifdef _DEBUG
int varGetRefCount() {
    return refCount;
}
#endif

void varDestroy(Variable *pVariable)
{
    if (pVariable == NULL)
        return;
    free(pVariable->value);
    free(pVariable);
#ifdef _DEBUG
    --refCount;
#endif
}

/* Private functions definitions */

// Compare

static int _cmpInteger(long long *a, long long *b)
{
    return *a == *b;
}

static int _cmpDecimal(double *a, double *b)
{
    return *a == *b;
}

static int _cmpString(char *a, char *b)
{
    return strcmp(a,b) == 0;
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