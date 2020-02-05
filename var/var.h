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
	
	/* Serialized form is corrupted */
	VAR_RETURN_FILE_FORMAT_ERROR,

	/* Could not write to file stream */
	VAR_RETURN_WRITING_ERROR,

	/* Could not allocate memory space */
	VAR_RETURN_MEMORY,
}
VarReturnID;

typedef struct Variable Variable;

/**
* Create a variable from parsing a text
* text        text to be parsed
* ppVariable  (return) pointer to variable
* Possible errors:
* VAR_RETURN_INVALID_PARAMETER
*   - "text" is NULL
*   - "ppVariable" is NULL
* VAR_RETURN_MEMORY
*/
VarReturnID varCreate(const char *text, Variable **ppVariable);

/**
* Create many variables by passing many texts
* text          text to be parsed
* ppVariable    (return) pointer to variable
* ...		    follows the same pattern
* NULL          ends the parameter list
* Possible errors:
* VAR_RETURN_INVALID_PARAMETER
*   - "text" is NULL
*   - "ppVariable" is NULL
* VAR_RETURN_MEMORY
*/
VarReturnID varCreateMultiple(const char *text, Variable **ppVariable, ...);

/**
* Compare two variables in respect to type and value
* pVariableA    first variable
* pVariableB    second variable
* pResult       (return) boolean return (0 = different, else, equal)
* Possible errors:
* VAR_RETURN_INVALID_PARAMETER
*   - "pVariableA" is NULL
*   - "pVariableB" is NULL
*   - "pResult" is NULL
*/
VarReturnID varCompare(Variable *pVariableA, Variable *pVariableB,
	int *pResult);

/**
* Write to file stream the contents of the variable
* pVariable	 variable
* fp         file pointer
* Possible errors:
* VAR_RETURN_INVALID_PARAMETER
*   - "pVariable" is NULL
* VAR_RETURN_WRITING_ERROR
* [!] The module does not take ownership of the file
* pointer (closing or opening it)
*/
VarReturnID varWrite(Variable *pVariable, FILE *fp);

/**
* Serializes variable to file
* pVariable	 variable
* fp         file pointer
* Possible errors:
* VAR_RETURN_INVALID_PARAMETER
*   - "pVariable" is NULL
* VAR_RETURN_WRITING_ERROR
* [!] The module does not take ownership of the file
* pointer (closing or opening it)
*/
VarReturnID varSerialize(Variable *pVariable, FILE *fp);

/**
* Deserializes file and creates variable object from it
* ppVariable	address of variable
* fp			file pointer
* VAR_RETURN_INVALID_PARAMETER
*   - "ppVariable" is NULL
* VAR_RETURN_FILE_FORMAT_ERROR
* VAR_RETURN_MEMORY
*/
VarReturnID varDeserialize(Variable **ppVariable, FILE *fp);

#ifdef _DEBUG
/* Not safe */
int varGetRefCount();
#endif

/**
* Safely dispose of variable
* pVariable	 variable
*/
void varDestroy(Variable *pVariable);

#endif