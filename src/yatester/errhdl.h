#ifndef yatester_errhdl_h
#define yatester_errhdl_h

/* Error Handler */

#include <yatester/err.h>

/**
 * @brief Push expected status
 * @param status expected status
 */
void yatester_pushexpectedstatus(yatester_status status);

/**
 * @brief Get expected status
 * @return expected status
 */
yatester_status yatester_getexpectedstatus();

/**
 * @brief Evaluates status in terms of expected status
 * @param Obtained status
 * @return Processed status
 */
yatester_status yatester_evaluatestatus(yatester_status status);

/**
 * @brief Pop expected status
 */
void yatester_popexpectedstatus();

#endif
