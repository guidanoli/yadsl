#ifndef __VAR_H__
#define __VAR_H__

#include <stdio.h>

/**
* A variable holds a generic structure that
* accompanies its type identifier, which is
* globally unique
*/

typedef enum
{
    /* All went ok */
    VAR_RETURN_OK = 0,

    /* Invalid parameter was provided */
    VAR_RETURN_INVALID_PARAMETER,
    
    /* Could not write to file stream */
    VAR_RETURN_WRITING_ERROR,

    /* Could not allocate memory space */
    VAR_RETURN_MEMORY,
}
VarReturnID;

typedef struct Variable Variable;

/**
* Create a variable from parsing a text
* text          text to be parsed
* ppVariable    (return) pointer to variable
* Possible errors:
* VAR_RETURN_INVALID_PARAMETER
*   - "text" is NULL
*   - "ppVariable" is NULL
* VAR_RETURN_MEMORY
*/
VarReturnID varCreate(const char *text, Variable **ppVariable);

/**
* Compare two variables in respect to type and value
* pVariableA    first variable
* pVariableB    second variable
* pResult       (return) boolean return (0 = equal, else, not)
* Possible errors:
* VAR_RETURN_INVALID_PARAMETER
*   - "pVariableA" is NULL
*   - "pVariableB" is NULL
*   - "pResult" is NULL
*/
VarReturnID varCompare(Variable *pVariableA, Variable *pVariableB, int *pResult);

/**
* Write to file stream the contents of the variable
* pVariable     variable
* fp            file pointer
* Possible errors:
* VAR_RETURN_INVALID_PARAMETER
*   - "pVariable" is NULL
* VAR_RETURN_WRITING_ERROR
* [!] The module does not take ownership of the file
* pointer (closing or opening it)
*/
VarReturnID varWrite(Variable *pVariable, FILE *fp);

#ifdef _DEBUG
/* Not safe */
int varGetRefCount();
#endif

void varDestroy(Variable *pVariable);

#endif