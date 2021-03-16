#ifndef yatester_err_h
#define yatester_err_h

/* Errors */

/**
 * @brief Status code
 */
typedef enum
{
	YATESTER_OK,  /**< No error */
	YATESTER_ERR, /**< Generic error */
	YATESTER_MEM, /**< Memory allocation error*/
	YATESTER_IO,  /**< I/O error */
}
yatester_status;

/**
 * @brief Throw status code
 * @note Performs a long jump
 */
void yatester_throw(yatester_status status);

/**
 * @brief Asserts condition is true
 * @note If not true, performs a long jump, throwing YATESTER_ERR.
 * @param code condition code
 * @param file file in which the assertion was made
 * @param line line in file in which assertion was made
 * @param condition condition being tested
 * @seealso yatester_assert
 * @seealso yatester_throw
 */
void yatester_assert_function(const char* code, const char* file, int line, int condition);

#define yatester_assert(condition) \
	yatester_assert_function(#condition, __FILE__, __LINE__, condition)

#endif
