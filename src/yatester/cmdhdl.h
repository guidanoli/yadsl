#ifndef yatester_cmdhdl_h
#define yatester_cmdhdl_h

/* Command Handler */

#include <yatester/cmd.h>
#include <yatester/status.h>

/**
 * @brief Initialize command handler
 * @note Before using the command handler, you must initialize it
 * @return operation status
 * @seealso yatester_terminatecmdhdl
 */
yatester_status yatester_initializecmdhdl();

/**
 * @brief Iterate through commands with a callback
 * @param callback function called with every registered command
 */
void yatester_itercommands(void (*callback)(const yatester_command*));

/**
 * @brief Get command by name
 * @param commandname command name
 * @return command or NULL on failure
 */
const yatester_command* yatester_getcommand(const char* commandname);

/**
 * @brief Terminate command handler
 * @note After using the command handler, you must terminate it
 * @seealso yatester_initializecmdhdl
 */
void yatester_terminatecmdhdl();

#endif
