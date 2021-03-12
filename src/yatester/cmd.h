#ifndef yatester_cmd_h
#define yatester_cmd_h

/* Commands */

typedef struct
{
	const char *name;
	int argc;
	void (*handler)(const char** argv);
}
yatester_command;

/* Terminates on command with name = NULL */
extern const yatester_command yatester_commands[];

#endif
