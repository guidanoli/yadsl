#ifndef yatester_errhdl_h
#define yatester_errhdl_h

/* Error Handler */

#include <yatester/err.h>

/**
 * @brief Evaluates status in terms of expected status
 * @param Obtained status
 * @return Processed status
 */
yatester_status yatester_evaluatestatus(yatester_status status);

#endif
