#ifndef yatester_err_h
#define yatester_err_h

/* Error Interface */

#include <yatester/status.h>

/**
 * @brief Raise error
 * @note Performs a long jump
 * @param status error status code
 * @note status != YATESTER_OK
 */
void yatester_raise(yatester_status status);

/**
 * @brief Assert condition is true
 * @note If not true, print a message and raise error
 * @param code condition code
 * @param file file in which the assertion was made
 * @param line line in file in which assertion was made
 * @param status error status code
 * @note status != YATESTER_OK
 * @param condition condition being tested
 * @seealso yatester_assert
 * @seealso yatester_raise
 */
void yatester_assert_function(const char* code, const char* file, int line, yatester_status status, int condition);

#define yatester_assert(status, condition) \
	yatester_assert_function(#condition, __FILE__, __LINE__, status, condition)

#endif
