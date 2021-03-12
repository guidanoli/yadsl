#ifndef yatester_err_h
#define yatester_err_h

/* Errors */

typedef enum
{
	YATESTER_OK  = 0,      /**< No error */
	YATESTER_ERR = 1 << 0, /**< Generic error */
	YATESTER_MEM = 1 << 1, /**< Memory allocation error*/
	YATESTER_IO  = 1 << 2, /**< I/O error */
}
yatester_status;

void yatester_throw(yatester_status status);

#endif
