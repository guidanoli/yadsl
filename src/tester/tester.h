#ifndef __YADSL_TESTER_H__
#define __YADSL_TESTER_H__

/**
 * \defgroup tester Tester
 * @brief C module testing framework
 *
 * This library incorporates the generic part of the testing framework, which
 * reads the test script and parses comands and its arguments.
 *
 * Motivation
 * ----------
 * Creating individual testers for each module can be a very dull and
 * error-prone task. It may lead to extra time wasted on debugging the test
 * module, but not the code itself. Having a single tester framework makes
 * testing effortless, enjoyable and energetic.
 *
 * Paradigm
 * --------
 * Instead of creating a test module entirely from scratch, you should only
 * write code that is specific to what you're testing, right? That is why you
 * only need to implement three functions, shown in order of activation.
 *
 * * yadsl_tester_init() - Called **once** to initialize data structures or global variables
 * * yadsl_tester_parse() - Called **for every command** parsed from the script file
 * * yadsl_tester_release() - Called **once** at the very end of the testing cycle
 *
 * You may also want to display some helping information if the tester receives
 * no script file, like what commands are available or how does your test work.
 * For that, you may also set the following variable:
 *
 * * yadsl_tester_help_strings - an array of strings terminated by `NULL`
 *
 * Scripts
 * -------
 * A script file has a very simple grammar, with the following tokens: commands,
 * arguments and comments. These tokens are separated by spaces, tabs and newlines.
 *
 * * **Commands** are written as / + string, preceding its arguments.
 * * **Arguments** can be parsed as int, char, float, long, size_t or char *.
 * * **Comments** are parsed as # + any string until the end of the line.
 *
 * Commands
 * --------
 * The tester framework calls the yadsl_tester_parse() for every potential command.
 * It is the job of this function to parse this string, and requiring more
 * parameters through yadsl_tester_parse_arguments(), if necessary.
 *
 * Return values
 * -------------
 * All of the implementable functions return an enumerator of type yadsl_TesterRet.
 * If any of these functions, when called, return a value other than ::YADSL_TESTER_RET_OK,
 * the corresponding cycle is interrupted. In the tester.h header are listed the
 * called "native" errors. These errors have their string identifiers too, which
 * are used for error handling, as we will see later on.
 *
 * External return values
 * ----------------------
 * You may have noticed that the EXTERNAL return value is the only one that does
 * not have a string nor a description defined. That is because it is a
 * polymorphic return value. It can be whatever the user determines at runtime.
 * Instead of returning this value right away, you ought to call
 * yadsl_tester_return_external_value(), passing its string as the parameter,
 * which then returns the enumerator.
 *
 * Error handling
 * --------------
 * There is only one way to catch errors raised by yadsl_tester_parse().
 * It is by calling the /catch command. Following the /catch command should be
 * the expected error string.
 *
 * Script native commands
 * ----------------------
 * ```
 * /catch <return>
 *   If <return> is equal to the last value returned, the error is ignored.
 *
 * /exit
 *   Exits tester environment. It is specially useful when on interactive mode.
 * ```
 *
 * Program arguments
 * -----------------
 * ```
 * Usage
 * -----
 *   <tester-executable> [<script-path>] [/I] [/LOG]
 *
 * Positional arguments
 * ---------------------
 *   script_path - Path to script file to be parsed
 *
 * Flag arguments
 * --------------
 *   /I - Interactive mode
 *     Reads from standard input instead of reading script
 *     (ignores script_path)
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
 * The syntax is specified below in EBNF.
 * Observation: some token matching depend on the implemention of the
 * C fscanf function family.
 *
 * ```
 * script ::= { line } ;
 * line ::= { cmd } , [ comment ] ;
 * cmd ::= '/' , cmd_name , { arg } ;
 * cmd_name ::= CHAR , { CHAR } ;
 * comment ::= '#' , { CHAR } ;
 * arg ::= CHAR | STRING | INTEGER | LONG | FLOAT | SIZE_T ;
 * CHAR ::= %c - '\n' - ' ' - '\t' ;
 * STRING ::= { CHAR } | '"' , { %c - '\n' - '"' } , '"' ;
 * INTEGER ::= %d | %u ;
 * LONG ::= %l | %lu ;
 * FLOAT ::= %f | %g | %e ;
 * SIZE_T ::= %zu ;
 * ```
 *
 * @{
*/


#include <stddef.h>
#include <stdbool.h>

#include <memdb/memdb.h>

/**
* @brief Enumeration of tester return values.
* OBS: ::YADSL_TESTER_RET_COUNT and ::YADSL_TESTER_RET_EXTERNAL are
* not meant to be used as return values, since they serve
* merely for internal purposes.
*/
typedef enum
{
	YADSL_TESTER_RET_OK = 0, /**< No errors occurred */
	YADSL_TESTER_RET_FILE, /**< Failed file operation */
	YADSL_TESTER_RET_MALLOC, /**< Failed memory allocation */
	YADSL_TESTER_RET_MEMLEAK, /**< Memory leak detected */
	YADSL_TESTER_RET_OVERFLOW, /**< Buffer overflow */
	YADSL_TESTER_RET_COMMAND, /**< Failed command parsing */
	YADSL_TESTER_RET_ARGUMENT, /**< Failed argument parsing */
	YADSL_TESTER_RET_TOKEN, /**< Failed token parsing */
	YADSL_TESTER_RET_RETURN, /**< Unexpected return */
	YADSL_TESTER_RET_CATCH, /**< Unexpected catch (no error) */
	YADSL_TESTER_RET_EXTERNAL, /**< External return value */
	YADSL_TESTER_RET_COUNT, /**< For internal use only */
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
extern const char* yadsl_tester_help_strings[];

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
extern yadsl_TesterRet yadsl_tester_parse(const char* command);

/**
 * @brief Callback called when yadsl_tester_parse returns
 * a value different from ::YADSL_TESTER_RET_OK, or after
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
int
yadsl_tester_parse_arguments(
	const char* format,
	...);

/**
 * @brief Parse a character that identifies the
 * object type, then parse the data type identified
 * by the character.
 * Examples: i 20, s "abc", c * 
 * If it could not parse the object, NULL is returned
 * and the state of the parser is then reversed to that
 * of before the call.
 * @return dynamically allocated object
 */
void*
yadsl_tester_object_parse();

/**
 * @brief Deallocates parsed object
 * @param object object parsed by tester
 */
void
yadsl_tester_object_free(
	void* object);

/**
 * @brief Check if two objects are equal
 * @param object1 first object
 * @param object2 second object
 * @return whether the two are equal
 */
bool
yadsl_tester_object_equal(
	void* object1,
	void* object2);

/**
 * @brief Copy object
 * @param object
 * @return object copy or NULL on failure
 */
void*
yadsl_tester_object_copy(
	void* object);

/**
 * @brief Get object data type
 * @param object 
 * @return object data type
*/
char
yadsl_tester_object_dtype(
	void* object);

/**
 * @brief Get object data
 * @param object
 * @return object data
*/
const void*
yadsl_tester_object_data(
	void* object);

/**
 * @brief Get size of data type
 * @param dtype data type identifier
 *
 * Observation: for strings, sizeof(char*) is returned.
 *
 * @return data type size or 0 if dtype is invalid
*/
size_t
yadsl_tester_get_dtype_size(
	char dtype);

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
int
yadsl_tester_compare_arguments(
	char dtype,
	const void* expected,
	const void* obtained);

/**
 * @brief Copy an argument from one location to another
 * @param dtype data type of both arguments
 * @param source argument being copied
 * @param destination where argument will be copied
*/
void
yadsl_tester_copy_argument(
	char dtype,
	const void* source,
	void* destination);

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
yadsl_TesterRet
yadsl_tester_return_external_value(
	const char* info);

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
void
yadsl_tester_log(
	const char* message,
	...);

/**
* @brief Prints help strings provided in the same
* way as if no arguments were provided.
*/
void
yadsl_tester_print_help_strings();

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
const char*
yadsl_tester_get_return_value_info(
	yadsl_TesterRet ret);

/** @} */

#endif
