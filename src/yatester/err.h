#ifndef yatester_err_h
#define yatester_err_h

/* Errors */

/**
 * @brief Status code
 */
typedef enum
{
	YATESTER_OK,        /**< Ok */
	YATESTER_ERROR,     /**< Error */
	YATESTER_NOMEM,     /**< No memory */
    YATESTER_FTLERR,    /**< Fatal error */
    YATESTER_MEMLK,     /**< Memory leak */
    YATESTER_MEMERR,    /**< Memory error */
	YATESTER_IOERR,     /**< I/O error */
    YATESTER_STXERR,    /**< Syntax error */
    YATESTER_CLARGERR,  /**< Command line argument error*/
    YATESTER_HELPCMD,   /**< Help command */
    YATESTER_LSTCMDS,   /**< List commands */
    YATESTER_NOCMD,     /**< No command */
    YATESTER_NOCMDHDL,  /**< No command handler */
    YATESTER_NOCMDNAME, /**< No command name */
    YATESTER_CMDNMCFLT, /**< Command name conflict */
    YATESTER_CMDARGCMM, /**< Command argument count mismatch */
    YATESTER_NOERROR,   /**< Expected error but didn't happen */
}
yatester_status;

/**
 * @brief Throw YATESTER_ERROR
 * @note Performs a long jump
 */
void yatester_throw();

/**
 * @brief Assert condition is true
 * @note If not true, prints a message and throws YATESTER_ERROR
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
 * @note if null, prints a message and throws YATESTER_NOMEM
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
