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
 * You may have noticed that the CUSTOM return value is the only one that does
 * not have a string nor a description defined. That is because it is a
 * polymorphic return value. It can be whatever the user determines at runtime.
 * Instead of returning this value right away, you ought to call
 * yadsl_tester_error(), passing its string as the parameter,
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
 * Program usage
 * -------------
 * 
 * ```
 * <program> [options...]
 * ```
 * 
 * If no arguments are provided, the program will commence reading from stdin.
 * 
 * Options
 * -------
 * 
 * ```
 * --help                         Print help message and quit
 *
 * --input-file <file-path>       Reads input from file.
 *                                (Default: standard input)
 * 
 * --log-file <file-path>         Logs debug information to file.
 *                                (Default: standard error)
 * 
 * --malloc-failing-rate <rate>   Changes memory allocation failing rate.
 *                                (Default: 0)
 * 
 * --malloc-failing-index         Sets memory allocation failing index
 *                                (Default: never fails by index)
 * 
 * --prng-seed <seed>             Changes pseudorandom number generator seed.
 *                                (Default: 0)
 * 
 * --enable-log-channel <channel> Enable log channel
 *                                (Default: all log channels are disabled)
 * ```
 * 
 * Log channels
 * ------------
 * 
 * * ALLOCATION, for memory allocation events
 * * DEALLOCATION, for memory deallocation events
 * * LEAKAGE, for memory leak events
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

#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef YADSL_DEBUG
#include <memdb/memdb.h>
#else
#include <stdlib.h>
#endif

/**
* @brief Enumeration of tester return values.
* OBS: ::YADSL_TESTER_RET_COUNT and ::YADSL_TESTER_RET_CUSTOM are
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
	YADSL_TESTER_RET_CUSTOM, /**< Custom error message */
	YADSL_TESTER_RET_COUNT, /**< For internal use only */
}
yadsl_TesterRet;

/*****************************************************************************/
/*                          SYMBOLS TO BE DEFINED                            */
/*****************************************************************************/

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

/*****************************************************************************/
/*                            DEFINED SYMBOLS                                */
/*****************************************************************************/

/**
 * @brief Parse arguments following the current command,
 * much like in scanf (altough the string format
 * is different).
 * Each character in the format string corresponds
 * to an argument to be parsed, in the same order.
 * The avaiable **data types** are:
 *
 * ```
 * +----------+------------------+----------------+
 * | C type   | Format character | Reference type |
 * +----------+------------------+----------------+
 * | float    | f                | float *        |
 * | int      | i                | int *          |
 * | long     | l                | long *         |
 * | char     | c                | char *         |
 * | char *   | s                | char [BUFSIZ]  |
 * | size_t   | z                | size_t *       |
 * | intmax_t | I                | intmax_t *     |
 * +----------+------------------+----------------+
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
 * @brief Throws an error number.
 * @param errnum error number
*/
void
yadsl_tester_throw_func(
	const char* errfile,
	int errline,
	yadsl_TesterRet errnum);

#define yadsl_tester_throw(...) \
	yadsl_tester_throw_func(__FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief Throws an error message.
 * Note: va_end(va) is called before doing
 * the long jump to avoid memory leakage
 * @param fmt error message format string
 * @param va format arguments
*/
void
yadsl_tester_vthrowf_func(
	const char* errfile,
	int errline,
	const char* fmt,
	va_list va);

#define yadsl_tester_vthrowf(...) \
	yadsl_tester_vthrowf_func(__FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief Throws an error message.
 * @param fmt error message format string
 * @param ... format arguments
*/
void
yadsl_tester_throwf_func(
	const char* errfile,
	int errline,
	const char* fmt,
	...);

#define yadsl_tester_throwf(...) \
	yadsl_tester_throwf_func(__FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief Restrict form of yadsl_tester_parse_arguments
 * Asserts that all arguments in format string match.
 * If not, peforms a long jump and an "argument" error is raised.
 * @param fmt format string
 * @param ... pointers to arguments
*/
#define yadsl_tester_parse_n_arguments(fmt, ...) \
	yadsl_tester_assert(yadsl_tester_parse_arguments(fmt, __VA_ARGS__) == \
		(sizeof(fmt)-1), YADSL_TESTER_RET_ARGUMENT)

/**
 * @brief Asserts that condition is true.
 * If not, performs a long jump and the error is raised.
 * @param condition condition to be tested
 * @param errnum error number
*/
void
yadsl_tester_assert_func(
	const char* errfile,
	int errline,
	int condition,
	yadsl_TesterRet errnum);

#define yadsl_tester_assert(...) \
	yadsl_tester_assert_func(__FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief Assert that condition is true.
 * If not, calls va_end(va), performs a long jump
 * and the error is raised
 * @param condition condition to be tester
 * @param fmt error message format string
 * @param va format arguments
*/
void
yadsl_tester_vassertf_func(
	const char* errfile,
	int errline,
	int condition,
	const char* fmt,
	va_list va);

#define yadsl_tester_vassertf(...) \
	yadsl_tester_vassertf_func(__FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief Assert that condition is true.
 * If not, performs a long jump and the error is raised
 * @param condition condition to be tester
 * @param fmt error message format string
 * @param ... format arguments
*/
void
yadsl_tester_assertf_func(
	const char* errfile,
	int errline,
	int condition,
	const char* fmt,
	...);

#define yadsl_tester_assertf(...) \
	yadsl_tester_assertf_func(__FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief Assert that condition is true.
 * If not, calls callback, va_end(va), performs a long jump
 * and the error is raised
 * @param condition condition to be tester
 * @param fmt error message format string
 * @param falsecb callback called when condition is false
 * @param va format arguments
*/
void
yadsl_tester_vxassertf_func(
	const char* errfile,
	int errline,
	int condition,
	const char* fmt,
	void (*falsecb)(),
	va_list va);

#define yadsl_tester_vxassertf(...) \
	yadsl_tester_vxassertf_func(__FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief Assert that condition is true.
 * If not, calls callback, performs a long jump
 * and the error is raised
 * @param condition condition to be tester
 * @param fmt error message format string
 * @param falsecb callback called when condition is false
 * @param .. format arguments
*/
void
yadsl_tester_xassertf_func(
	const char* errfile,
	int errline,
	int condition,
	const char* fmt,
	void (*falsecb)(),
	...);

#define yadsl_tester_xassertf(...) \
	yadsl_tester_xassertf_func(__FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief Asserts floats are equal
*/
void
yadsl_tester_asserteqf_func(
	const char* errfile,
	int errline,
	float a,
	float b,
	const char* errmsg);

#define yadsl_tester_asserteqf(...) \
	yadsl_tester_asserteqf_func(__FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief Asserts integers are equal
*/
void
yadsl_tester_asserteqi_func(
	const char* errfile,
	int errline,
	int a,
	int b,
	const char* errmsg);

#define yadsl_tester_asserteqi(...) \
	yadsl_tester_asserteqi_func(__FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief Asserts longs are equal
*/
void
yadsl_tester_asserteql_func(
	const char* errfile,
	int errline,
	long a,
	long b,
	const char* errmsg);

#define yadsl_tester_asserteql(...) \
	yadsl_tester_asserteql_func(__FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief Asserts characters are equal
*/
void
yadsl_tester_asserteqc_func(
	const char* errfile,
	int errline,
	char a,
	char b,
	const char* errmsg);

#define yadsl_tester_asserteqc(...) \
	yadsl_tester_asserteqc_func(__FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief Asserts strings are equal
*/
void
yadsl_tester_asserteqs_func(
	const char* errfile,
	int errline,
	const char* a,
	const char* b,
	const char* errmsg);

#define yadsl_tester_asserteqs(...) \
	yadsl_tester_asserteqs_func(__FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief Asserts size types are equal
*/
void
yadsl_tester_asserteqz_func(
	const char* errfile,
	int errline,
	size_t a,
	size_t b,
	const char* errmsg);

#define yadsl_tester_asserteqz(...) \
	yadsl_tester_asserteqz_func(__FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief Asserts max. integers are equal
*/
void
yadsl_tester_asserteqI_func(
	const char* errfile,
	int errline,
	intmax_t a,
	intmax_t b,
	const char* errmsg);

#define yadsl_tester_asserteqI(...) \
	yadsl_tester_asserteqI_func(__FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief Create an object
 * @param dtype data type identifier
 * @param data object data
 * @return newly created object or NULL on failure
*/
void*
yadsl_tester_object_create(
	char dtype,
	char* data);

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
 * @brief Get format for data type
 * @param dtype data type identifier
 * @return format for C stdio.h functions or 0 if dtype is invalid
*/
const char*
yadsl_tester_get_dtype_fmt(
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
 * @brief Create a custom error message.
 *
 * Hint
 * ----
 * The error can be caught by '/catch' too,
 * by providing the same error message.
 *
 * @param errmsg error message
 * @return ::YADSL_TESTER_RET_CUSTOM
*/
yadsl_TesterRet
yadsl_tester_error_func(
	const char* errfile,
	int errline,
	const char* errmsg);

#define yadsl_tester_error(errmsg) \
	yadsl_tester_error_func(__FILE__, __LINE__, errmsg)

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

/** @} */

#endif
