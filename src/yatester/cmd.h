#ifndef yatester_cmd_h
#define yatester_cmd_h

/* Commands */

/**
 * @brief Argument count restriction macros
 * Where 0 <= argc <= INT_MAX
 */
#define AT_LEAST(argc) (argc)
#define AT_MOST(argc) (-(argc)-1)

/**
 * @brief Command definition
 * @note Use AT_LEAST and AT_MOST for the argc field
 */
typedef struct
{
	const char *name; /**< Command name */
	int argc; /**< Minimum/maximum number of arguments */
	void (*handler)(int argc, char** argv); /**< Command handler */
}
yatester_command;

/**
 * @brief Command table
 * @note The last command must have name = NULL
 */
extern const yatester_command yatester_commands[];

#endif
