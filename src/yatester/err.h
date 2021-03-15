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

#endif
