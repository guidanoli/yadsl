#ifndef yatester_status_h
#define yatester_status_h

/* Status */

#include <stdarg.h>

/**
 * @brief Status code
 */
typedef enum
{
	YATESTER_OK, /**< Ok */
	YATESTER_ERROR, /**< Error */
	YATESTER_NOMEM, /**< No memory */
	YATESTER_FTLERR, /**< Fatal error */
	YATESTER_MEMLK, /**< Memory leak */
	YATESTER_IOERR, /**< I/O error */
	YATESTER_STXERR, /**< Syntax error */
	YATESTER_BADCMD, /**< Bad command */
	YATESTER_BADCALL, /**< Bad call */
	YATESTER_BADARG, /**< Bad argument */
}
yatester_status;

/**
 * @brief Report error
 * @note Prints to standard error
 * @param status error status
 * @param fmt error message format
 * @param ... format arguments
 * @return status
 */
yatester_status yatester_report(yatester_status status, const char* fmt, ...);

/**
 * @brief Report error
 * @note Prints to standard error
 * @param status error status
 * @param fmt error message format
 * @param va format arguments
 * @return status
 */
yatester_status yatester_vreport(yatester_status status, const char* fmt, va_list va);

#endif
