#ifndef __YADSL_ARGVP_H__
#define __YADSL_ARGVP_H__

/**
 * \defgroup argvp ARGV Parser
 * 
 * @brief Utility module for parsing argument vectors.
 * @{
*/

/**
 * @brief Keyword argument definition
*/
typedef struct
{
	const char* kw; /**< Keyword (e.g. "--verbose") */
	int valc; /**< Number of values that come after */
}
yadsl_ArgvKeywordArgumentDef;

typedef void yadsl_ArgvParserHandle; /**< Argument vector parser handle */

/**
 * @brief Create a argument vector parser
 * @param argc argument count
 * @param argv argument vector
 * @return newly created argument vector parser or NULL on failure
 * @see yadsl_argvp_destroy
*/
yadsl_ArgvParserHandle*
yadsl_argvp_create(
	int argc,
	char** argv);

/**
 * @brief Add keyword arguments from array
 * @param argvp argument vector parser
 * @param kwargdefs NULL-terminated array of keyword argument definitions
*/
void
yadsl_argvp_add_keyword_arguments(
	yadsl_ArgvParserHandle* argvp,
	yadsl_ArgvKeywordArgumentDef kwargdefs[]);

/**
 * @brief Add a keyword argument
 * @param argvp argument vector parser
 * @param kw keyword argument name
 * @param valc number of values associated with the keyword argument
 * @see yadsl_argvp_get_keyword_argument_value
 * @see yadsl_argvp_parse_keyword_argument_value
*/
void
yadsl_argvp_add_keyword_argument(
	yadsl_ArgvParserHandle* argvp,
	const char* kw,
	int valc);

/**
 * @brief Get positional argument count
 * @param argvp argument vector parser
 * @return number of positional arguments
 * @see yadsl_argvp_get_positional_argument
*/
int
yadsl_argvp_get_positional_argument_count(
	yadsl_ArgvParserHandle* argvp);

/**
 * @brief Get positional argument
 * @param argvp argument vector parser
 * @param argi argument index
 * @return positional argument or NULL if the index is invalid
 * @see yadsl_argvp_get_positional_argument_count
*/
const char*
yadsl_argvp_get_positional_argument(
	yadsl_ArgvParserHandle* argvp,
	int argi);

/**
 * @brief Parse positional argument
 * @param argvp argument vector parser
 * @param argi argument index
 * @param fmt format string passed to `vsscanf`
 * @param ... varadic arguments passed to `vsscanf`
 * @return return value of `vsscanf` or 0 if the index is invalid
 * @see yadsl_argvp_get_positional_argument_count
 * @see yadsl_argvp_get_positional_argument
*/
int
yadsl_argvp_parse_positional_argument(
	yadsl_ArgvParserHandle* argvp,
	int argi,
	const char* fmt,
	...);

/**
 * @brief Get keyword argument value
 * @param argvp argument vector parser
 * @param kw keyword argument name
 * @param vali value index
 * @return keyword argument or NULL if the index is invalid or
 * if the keyword argument doesn't exist
 * @see yadsl_argvp_add_keyword_argument
*/
const char*
yadsl_argvp_get_keyword_argument_value(
	yadsl_ArgvParserHandle* argvp,
	const char* kw,
	int vali);

/**
 * @brief Parse keyword argument value
 * @param argvp argument vector parser
 * @param kw keyword argument name
 * @param vali value index
 * @param fmt format string passed to `vsscanf`
 * @param ... varadic arguments passed to `vsscanf`
 * @return return value of `vsscanf` or 0 if the index is invalid or
 * if the keyword argument doesn't exist
 * @see yadsl_argvp_add_keyword_argument
 * @see yadsl_argvp_get_keyword_argument_value
*/
int
yadsl_argvp_parse_keyword_argument_value(
	yadsl_ArgvParserHandle* argvp,
	const char* kw,
	int vali,
	const char* fmt,
	...);

/**
 * @brief Check if keyword argument exists
 * @param argvp argument vector parser
 * @param kw keyword argument name
 * @return whether keyword argument exists or not
 * @see yadsl_argvp_add_keyword_argument
 * @see yadsl_argvp_add_keyword_arguments
*/
int
yadsl_argvp_has_keyword_argument(
    yadsl_ArgvParserHandle* argvp,
    const char* kw);

/**
 * @brief Destroy argument vector parser
 * @param argvp argument vector parser
 * @see yadsl_argvp_create
*/
void
yadsl_argvp_destroy(
	yadsl_ArgvParserHandle* argvp);

/** @} */

#endif
