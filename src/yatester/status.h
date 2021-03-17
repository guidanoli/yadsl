#ifndef yatester_status_h
#define yatester_status_h

/* Status code */

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

#endif
