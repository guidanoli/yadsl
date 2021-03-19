#ifndef yatester_cmd_h
#define yatester_cmd_h

/* Commands */

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

/* Terminates on command with name = NULL */
extern const yatester_command yatester_commands[];

#endif
