#ifndef __TESTER_H__
#define __TESTER_H__

#include <stdio.h>

/**********************************************
* HINT: Lines are limited to TESTER_BUFFER_SIZE
***********************************************/

#define TESTER_BUFFER_SIZE 1024

/**********************************************
* TESTER FRAMEWORK
* 
* This incorporates the generic part of the
* tester framework, which reads the script,
* and parses commands and its arguments.
*
* SCRIPT GRAMMAR DEFINITION
*
* script: line | script line
* line: command | command comment | sep
* command: cmdname | command sep cmdarg
* cmdarg: float | string | integer
* float: %f | %g | %e
* string: string_qm | string_wo_qm
* integer: %d | %u
*
* SCRIPT TOKENS DEFINITIONS
*
* sep = [ \t\n]+
* comment = #.*
* cmdname = /[^ \t\n]+
* string_qm = "[^"]*"
* string_wo_qm = [^ \t\n]+
* % identifiers from C standard library
*
* NATIVE COMMANDS
*
* /catch <return-value>
*   if return-value equals the last command
*   return value, then, it is ignored. else,
*   an error will be thrown.
***********************************************/

/**
* Enumeration of all possible return values intelligible
* to the Tester framework. You can import external return
* values to customize even more your test by defining the
* path to the file with a list of comma-terminated symbols
* to TESTER_EXTERNAL_RETURN_VALUES.
* You can even add further information about the return
* value by implementing the TesterLoadCustomReturnValueInfo.
* HINT: External return values will be numerated beggining
* from 1. That way you can use the correct value for the 
* /catch command by looking up the position of a given
* return value on your definitions file.
* HINT: TESTER_RETURN_COUNT and TESTER_RETURN_FLAG are not
* meant to be used as return values, since it serves merely
* for internal purposes.
*/
typedef enum
{
    TESTER_RETURN_OK = 0,
#ifdef TESTER_EXTERNAL_RETURN_VALUES
    #include TESTER_EXTERNAL_RETURN_VALUES
#endif
    TESTER_RETURN_FLAG,
    TESTER_RETURN_FILE,
    TESTER_RETURN_MEMORY_LACK,
    TESTER_RETURN_MEMORY_LEAK,
    TESTER_RETURN_BUFFER_OVERFLOW,
    TESTER_RETURN_PARSING_COMMAND,
    TESTER_RETURN_PARSING_ARGUMENT,
    TESTER_RETURN_UNEXPECTED_ARGUMENT,
    TESTER_RETURN_UNEXPECTED_RETURN,
    TESTER_RETURN_COUNT,
}
TesterReturnValue;

/**********************************************
* SYMBOLS YOU MUST DEFINE
*
* The specific functions that must be defined
* can return TesterReturnValue. If a value other
* than TESTER_RETURN_OK is returned, the test
* is interrupted, returning that value.
***********************************************/

/**
* When no arguments are provided, help strings
* can be displayed to the user to show how to
* use the test module commands correctly.
* In your test source code, define this as
* an array of strings, terminated with a
* NULL, necessarily. But, if you do not want
* this feature, simply assign it to NULL.
*/
extern const char *TesterHelpStrings[];

/**
* Callback Tester calls after opening the script
* file successfully, in order to the caller to
* initialize its variables properly before parsing.
*/
extern TesterReturnValue TesterInitCallback();

/**
* Callback called when Tester finds a command
* call. It is your job to parse the command and
* require aditional arguments, if needed.
*/
extern TesterReturnValue TesterParseCallback(const char *command);

/**
* Callback called when TesterParseCallback returns
* a value different from TESTER_RETURN_OK, or after
* parsing the whole script, if no errors are thrown.
*/
extern TesterReturnValue TesterExitCallback();

/**
* Callback called when Tester return values information
* are being loaded and custom values are also defined,
* but whose information must be loaded too.
* HINT: It is advised to return NULL by default, so
* that the proper information is assigned to return
* values with missing information.
*/
#ifdef TESTER_EXTERNAL_RETURN_VALUES
extern const char *TesterLoadCustomReturnValueInfo(TesterReturnValue value);
#endif

/**********************************************
* SYMBOLS ALREADY DEFINED
***********************************************/

/**
* Parse arguments following the current command,
* much like in scanf, altough the string format
* is different, and return the number of arguments
* parsed successfully.
* Each character in the format string corresponds
* to an argument to be parsed, in that order:
*   'f' .................... float
*   'i' .................... integer
*   's' .................... string (*)
* Following the format string, must be passed by
* reference the argument variables.
* If it fails to parse one of the argument, -1 is
* returned, but the state of the parser is then
* reversed to that of before the call.
* (*) Must be of at least the size of the buffer
* (*) Can parse strings within quotation marks too.
*/
int TesterParseArguments(const char *format, ...);

/**
* Prints help strings provided in the same
* way as if no arguments were provided.
* HINT: stdout and stderr are FILE * too.
*/
void TesterPrintHelpStrings(FILE *fp);

/**
* Get further information about return value
* HINT: If an invalid return value is given,
* a proper error message is returned.
*/
const char *TesterGetReturnValueInfo(TesterReturnValue returnValue);

#endif
