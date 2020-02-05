#ifndef __TESTER_H__
#define __TESTER_H__

////////////////////////////////////////////////////////////////////////////////
// TESTER FRAMEWORK
// 
// This incorporates the generic part of the
// tester framework, which reads the script,
// and parses commands and its arguments.
//
// SCRIPT GRAMMAR DEFINITION
//
// script: line | script nl line
// line: lsep lcmds lcomm | $
// lsep: lsep sep | $
// lcmds: lcmds cmd | $
// lcomm: comm | $
// cmd: cmdname | cmdname largs
// largs: largs lsep sep arg
// arg: float | string | integer | long
// float: %f | %g | %e
// string: string_qm | string_wo_qm
// integer: %d | %u
// long: %ld | %lu
//
// SCRIPT TOKENS DEFINITIONS
//
// $ = 
// nl = \n
// sep = [ \t]
// comment = #[^\n]*
// cmdname = /[^ \t\n]+
// string_qm = "[^"\n]*"
// string_wo_qm = [^ \t\n]+
// % identifiers from C standard library
//
// NATIVE COMMANDS
//
// /catch <return>
//   if <return> is equal to the last value
//   returned, the error is ignored.
////////////////////////////////////////////////////////////////////////////////

/**
* Enumeration of tester return values.
* OBS: TESTER_RETURN_COUNT and TESTER_RETURN_EXTERNAL are
* not meant to be used as return values, since they serve
* merely for internal purposes.
*/
typedef enum
{
	TESTER_RETURN_OK = 0,
	TESTER_RETURN_FILE,
	TESTER_RETURN_MALLOC,
	TESTER_RETURN_MEMLEAK,
	TESTER_RETURN_OVERFLOW,
	TESTER_RETURN_COMMAND,
	TESTER_RETURN_ARGUMENT,
	TESTER_RETURN_RETURN,
	TESTER_RETURN_EXTERNAL,
	TESTER_RETURN_COUNT,
}
TesterReturnValue;

////////////////////////////////////////////////////////////////////////////////
// SYMBOLS YOU MUST DEFINE
//
// The specific functions that must be defined
// can return TesterReturnValue. If a value other
// than TESTER_RETURN_OK is returned, the test
// is interrupted, returning that value.
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

////////////////////////////////////////////////////////////////////////////////
// SYMBOLS ALREADY DEFINED
////////////////////////////////////////////////////////////////////////////////

/**
* Parse arguments following the current command,
* much like in scanf, altough the string format
* is different, and return the number of arguments
* parsed successfully.
* Each character in the format string corresponds
* to an argument to be parsed, in that order:
*   'f' .................... float
*   'i' .................... integer
*   'l' .................... long
*   's' .................... string (*)
* Following the format string, must be passed by
* reference the argument variables.
* If it fails to parse one of the argument, -1 is
* returned, but the state of the parser is then
* reversed to that of before the call.
* (*) Must be of at least the size of BUFSIZ (stdio.h)
* (*) Can parse strings within quotation marks too.
*/
int TesterParseArguments(const char *format, ...);

/**
* Create an external value for an specific error.
* Usage: "return TesterExternalReturnValue("myerror");"
* HINT: The external value can be caught by '/catch' too.
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
