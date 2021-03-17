#ifndef yatester_err_h
#define yatester_err_h

/* Errors */

/**
 * @brief Throw error
 * @note Performs a long jump
 */
void yatester_throw();

/**
 * @brief Assert condition is true
 * @note If not true, prints a message and throws error
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

/**
 * @brief Assert pointer is not null
 * @note If null, prints a message and throws memory error
 * @param code pointer code
 * @param file file in which function was called
 * @param line line in file in which function was called
 * @param p pointer
 * @seealso yatester_notnull
 */
void yatester_notnull_function(const char* code, const char* file, int line, void* p);

#define yatester_notnull(p) \
	yatester_notnull_function(#p, __FILE__, __LINE__, p)

#endif
