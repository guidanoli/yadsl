#ifndef __TESTER_H__
#define __TESTER_H__

//    ______          __           
//   /_  __/__  _____/ /____  _____
//    / / / _ \/ ___/ __/ _ \/ ___/
//   / / /  __(__  ) /_/  __/ /    
//  /_/  \___/____/\__/\___/_/                                 
//
// This incorporates the generic part of the tester framework, which reads
// the script, and parses commands and its arguments.
//
// COMMAND LINE ARGUMENTS
// ======================
//
// The executable takes the following arguments:
// $ testerexecutable [script_path] [/I] [/LOG]
//
// Positional arguments
// ---------------------
// script_path: path to script file to be parsed
//
// Flag arguments
// --------------
// /I: interactive mode
//   Ignores script_path, reads from stdin
//
// /LOG: logs debug information
//   Redirects debugging output to file other than stdout
//   e.g. memdb information goes to memdb.log
//
// No arguments
// ------------
// If no arguments are given, a help message will be displayed.
//
// SCRIPT GRAMMAR DEFINITION
// =========================
//
// script: line | script nl line
// line: osep lcmds sep comment osep | osep lcmds osep
// lcmds: lcmds sep cmdname largs | $
// largs: largs sep arg | $
// arg: float | string | integer | long | size
// float: %f | %g | %e
// string: string_qm | string_wo_qm
// integer: %d | %u
// long: %ld | %lu
// size: %zu
//
// SCRIPT TOKENS DEFINITIONS
// =========================
//
// $ = 
// nl = \n
// sep = [ \t]+
// osep = [ \t]*
// comment = #[^\n]*
// cmdname = /[^ \t\n]+
// string_qm = "[^"\n]*"
// string_wo_qm = [^ \t\n]+
// % identifiers from C standard library
//
// NATIVE COMMANDS
// ===============
//
// /catch <return>
//   if <return> is equal to the last value
//   returned, the error is ignored.
//
// /exit
//   exits tester environment
//   specially useful when on interactive mode
//

#include <stddef.h>

#include <memdb/memdb.h>

/**
* Enumeration of tester return values.
* OBS: TESTER_COUNT and TESTER_EXTERNAL are
* not meant to be used as return values, since they serve
* merely for internal purposes.
*/
typedef enum
{
	TESTER_OK = 0,
	TESTER_FILE,
	TESTER_MALLOC,
	TESTER_MEMLEAK,
	TESTER_OVERFLOW,
	TESTER_COMMAND,
	TESTER_ARGUMENT,
	TESTER_RETURN,
	TESTER_EXTERNAL,
	TESTER_COUNT,
}
TesterReturnValue;

////////////////////////////////////////////////////////////////////////////////
// SYMBOLS YOU MUST DEFINE
////////////////////////////////////////////////////////////////////////////////

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
* file successfully, allowing the caller to
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
* a value different from TESTER_OK, or after
* parsing the whole script, if no errors are thrown.
*/
extern TesterReturnValue TesterExitCallback();

////////////////////////////////////////////////////////////////////////////////
// SYMBOLS ALREADY DEFINED
////////////////////////////////////////////////////////////////////////////////

/**
* Parse arguments following the current command,
* much like in scanf (altough the string format
* is different). Returns the number of arguments
* parsed successfully.
* Each character in the format string corresponds
* to an argument to be parsed, in the same order.
* The avaiable argument types are:
*
* +--------+------------------+----------------+
* | C type | Format character | Reference type |
* +--------+------------------+----------------+
* | float  | f                | float *        |
* | int    | i                | int *          |
* | long   | l                | long *         |
* | char   | c                | char *         |
* | char * | s                | char * buffer  |
* | size_t | z                | size_t *       |
* +--------+------------------+----------------+
*
* Following the format string, must be passed by
* reference the argument variables in the appropriate
* reference type, as shown on the table above.
* If it fails to parse one of the argument, -1 is
* returned, and the state of the parser is then
* reversed to that of before the call.
*
* Observations about strings:
* - Buffer must be of at least the size of BUFSIZ,
*   defined in stdio.h.
* - Can parse strings within quotation marks too,
*   ignoring separation characters.
*/
int TesterParseArguments(const char *format, ...);

/**
 * Get size of accepted data type by Tester
 * Returns 0 if dtype is invalid.
*/
size_t TesterGetDataTypeSize(char dtype);

/**
 * Check if the arguments of type identified by the 'dtype' are equal.
 * Returns 0 if and only if they are equal.
*/
int TesterCompare(char dtype, const void* expected, const void* obtained);

/**
 * Copy data from source to destination, where source has
 * data type 'dtype'.
*/
void TesterCopy(char dtype, const void* source, void* destination);

/**
* Create an external value for an specific error.
* Once called, its return value must be returned by the
* callback from which it was called.
* HINT: The external value can be caught by '/catch' too,
* by providing the same string 'info' as its parameter.
*/
TesterReturnValue TesterExternalReturnValue(const char *info);

/**
* Log a message with additional information about current
* parser state and cursor position, wrapping fprintf.
*/
void TesterLog(const char *message, ...);

/**
* Prints help strings provided in the same
* way as if no arguments were provided.
*/
void TesterPrintHelpStrings();

/**
* Get further information about return value
* HINT: If an invalid return value is given,
* a proper error message is returned.
*/
const char *TesterGetReturnValueInfo(TesterReturnValue returnValue);

#endif
