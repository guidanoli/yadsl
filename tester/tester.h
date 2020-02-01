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
* /catch [ext] <return-value>
*   if return-value equals the last command
*   return value, then, it is ignored. else,
*   an error will be thrown.
*   the 'ext' flag is optional, for
*   catching external return values
***********************************************/

/**
* Enumeration of all possible return values intelligible
* to the Tester framework.
* OBS: TESTER_RETURN_COUNT is not meant to be used as return
* values, since it serves merely for internal purposes.
* OBS: TESTER_RETURN_EXTERNAL is not meant to be used directly.
* Use the TesterExternalValue function instead.
*/
typedef enum
{
    TESTER_RETURN_OK = 0,
    TESTER_RETURN_FILE,
    TESTER_RETURN_MEMORY_LACK,
    TESTER_RETURN_MEMORY_LEAK,
    TESTER_RETURN_BUFFER_OVERFLOW,
    TESTER_RETURN_PARSING_COMMAND,
    TESTER_RETURN_PARSING_ARGUMENT,
    TESTER_RETURN_UNEXPECTED_ARGUMENT,
    TESTER_RETURN_UNEXPECTED_RETURN,
    TESTER_RETURN_EXTERNAL,
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
* Create an external value (and, optionally, information)
* for an specific error occurred during TesterParseCallback.
* Its return should be the return value of TesterParseCallback,
* i.e. "return TesterExternalValue(404, "Page not found");"
* HINT: The external value can be caught by /catch ext <val>
*/
TesterReturnValue TesterExternalValue(int value, const char *info);

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
