#ifndef yatester_err_h
#define yatester_err_h

/* Errors */

typedef enum
{
	YATESTER_OK,  /**< No error */
	YATESTER_ERR, /**< Generic error */
	YATESTER_MEM, /**< Memory allocation error*/
	YATESTER_IO,  /**< I/O error */
}
yatester_status;

void yatester_throw(yatester_status status);

void yatester_assert_function(const char* code, const char* file, int line, int condition);

#define yatester_assert(condition) \
	yatester_assert_function(#condition, __FILE__, __LINE__, condition)

#endif
