#ifndef __YADSL_TESTER_RET_H__
#define __YADSL_TESTER_RET_H__

/**
 * \defgroup tester Tester
 * @brief C module testing framework
 *
 * This library incorporates the generic part of the testing framework, which
 * reads the test script and parses comands and its arguments.
 * 
 * Program arguments
 * -----------------
 * 
 * ```
 * Usage
 * -----
 *   <tester-executable> [<script-path> [/I] [/LOG]]
 *
 * Positional arguments
 * ---------------------
 *   script_path - Path to script file to be parsed
 *
 * Flag arguments
 * --------------
 *   /I - Interactive mode
 *     Reads from standard input instead of reading script
 *
 *   /LOG - Logs debug information
 *     Redirects debugging output to file other than the standard output
 *     e.g. memdb information goes to memdb.log
 *
 * No arguments
 * ------------
 *   If no arguments are given, a help message will be displayed, if available.
 * ```
 * 
 * Script grammar
 * --------------
 *
 * The context free language is specified below:
 * 
 * ```
 * Script -> Line | Script "\n" Line
 * Line -> sep* CmdList sep+ comment sep* | sep* CmdList sep*
 * CmdList -> CmdList sep+ cmdname ArgList | epsilon
 * ArgList -> ArgList sep+ Arg | epsilon
 * Arg -> Float | String | Integer | Long | Size | Char
 * Float -> %f | %g | %e
 * String -> string_qm | string_wo_qm
 * Integer -> %d | %u
 * Long -> %ld | %lu
 * Size -> %zu
 * Char -> %c
 * ```
 *
 * Script tokens
 * -------------
 *
 * All the tokens are specified below as regular expressions:
 * 
 * ```
 * sep = [ \t]
 * comment = #[^\n]*
 * cmdname = /[^ \t\n]+
 * string_qm = "[^"\n]*"
 * string_wo_qm = [^ \t\n]+
 * %? = identifiers from C standard library
 * ```
 * 
 * Script native commands
 * ----------------------
 *
 * ```
 * /catch <return>
 *   If <return> is equal to the last value returned, the error is ignored.
 *
 * /exit
 *   Exits tester environment. It is specially useful when on interactive mode.
 * ```
 * 
 * @{
*/


#include <stddef.h>

#include <memdb/memdb.h>

/**
* @brief Enumeration of tester return values.
* OBS: YADSL_TESTER_RET_COUNT and YADSL_TESTER_RET_EXTERNAL are
* not meant to be used as return values, since they serve
* merely for internal purposes.
*/
typedef enum
{
	YADSL_TESTER_RET_OK = 0,
	YADSL_TESTER_RET_FILE,
	YADSL_TESTER_RET_MALLOC,
	YADSL_TESTER_RET_MEMLEAK,
	YADSL_TESTER_RET_OVERFLOW,
	YADSL_TESTER_RET_COMMAND,
	YADSL_TESTER_RET_ARGUMENT,
	YADSL_TESTER_RET_RETURN,
	YADSL_TESTER_RET_PARSING,
	YADSL_TESTER_RET_EXTERNAL,
	YADSL_TESTER_RET_COUNT,
}
yadsl_TesterRet;

/***************************/
/* SYMBOLS YOU MUST DEFINE */
/***************************/

/**
 * @brief When no arguments are provided, help strings
 * can be displayed to the user to show how to
 * use the test module commands correctly.
 * In your test source code, define this as
 * an array of strings, terminated with a
 * NULL, necessarily. But, if you do not want
 * this feature, simply assign it to NULL.
*/
extern const char *yadsl_tester_help_strings[];

/**
 * @brief Callback called after opening the script
 * file successfully, allowing the caller to
 * initialize its variables properly before parsing.
 * @return status (if not ok, aborts)
*/
extern yadsl_TesterRet yadsl_tester_init();

/**
 * @brief Callback called for every command in the script.
 * It is your job to require aditional arguments, if needed.
 * @param command command name (i.e. without / or separators)
 * @return status (if not ok, and the error is not caught, aborts)
*/
extern yadsl_TesterRet yadsl_tester_parse(const char *command);

/**
 * @brief Callback called when yadsl_tester_parse returns
 * a value different from YADSL_TESTER_RET_OK, or after
 * parsing the whole script, if no errors are thrown.
 * @return status (if not ok, aborts)
*/
extern yadsl_TesterRet yadsl_tester_release();

/***************************/
/* SYMBOLS ALREADY DEFINED */
/***************************/

/**
 * @brief Parse arguments following the current command,
 * much like in scanf (altough the string format
 * is different).
 * Each character in the format string corresponds
 * to an argument to be parsed, in the same order.
 * The avaiable **data types** are:
 *
 * ```
 * +--------+------------------+----------------+
 * | C type | Format character | Reference type |
 * +--------+------------------+----------------+
 * | float  | f                | float *        |
 * | int    | i                | int *          |
 * | long   | l                | long *         |
 * | char   | c                | char *         |
 * | char * | s                | char [BUFSIZ]  |
 * | size_t | z                | size_t *       |
 * +--------+------------------+----------------+
 * ```
 * 
 * Following the format string, the argument variables
 * must be passed by reference in the same order and in
 * the appropriate type, as shown on the table above.
 * If it fails to parse one of the argument, -1 is
 * returned, and the state of the parser is then
 * reversed to that of before the call.
 *
 * Observations about strings:
 * - Buffers must be of at least the size of BUFSIZ,
 *   a macro defined in the stdio.h header file.
 * - Strings between quotation marks can be parsed too,
 *   allowing separation characters to be ignored.
 * 
 * @param format format string
 * @param ... pointers to arguments
 * @return number of successfully parsed arguments
*/
int yadsl_tester_parse_arguments(const char *format, ...);

/**
 * @brief Get size of data type
 * @param dtype data type identifier
 * @return data type size or 0 if dtype is invalid
*/
size_t yadsl_tester_get_dtype_size(char dtype);

/**
 * @brief Compare two arguments of same data type
 * @param dtype data type of both arguments
 * @param expected first argument
 * @param obtained second argument
 * @return
 * * 0 if the arguments are equal
 * * > 0 if the first argument is greater
 * * < 0 if the second argument is greater
*/
int yadsl_tester_compare_arguments(char dtype, const void* expected, const void* obtained);

/**
 * @brief Copy an argument from one location to another
 * @param dtype data type of both arguments
 * @param source argument being copied
 * @param destination where argument will be copied
*/
void yadsl_tester_copy_argument(char dtype, const void* source, void* destination);

/**
 * @brief Create an external value for an specific error.
 * Once called, its return value must be returned by the
 * callback from which it was called.
 * 
 * Hint
 * ----
 * The external value can be caught by '/catch' too,
 * by providing the same string 'info' as its parameter.
 * 
 * @param info external value name
 * @return ::YADSL_TESTER_RET_EXTERNAL
*/
yadsl_TesterRet yadsl_tester_return_external_value(const char *info);

/**
 * @brief Log a message with additional information about current
 * parser state and cursor position, by wrapping fprintf.
 * 
 * Hint
 * ----
 * Automatically jumps a line.
 * 
 * @param message format string, similar to fprintf.
 * @param ... varadic arguments, similar to fprintf.
*/
void yadsl_tester_log(const char *message, ...);

/**
* @brief Prints help strings provided in the same
* way as if no arguments were provided.
*/
void yadsl_tester_print_help_strings();

/**
 * @brief Get further information about return value
 * 
 * Hint
 * ----
 * If an invalid return value is given, a proper error message is returned.
 * 
 * @param ret return value
 * @return information about return value or error message, if invalid.
*/
const char *yadsl_tester_get_return_value_info(yadsl_TesterRet ret);

/** }@ */

#endif
