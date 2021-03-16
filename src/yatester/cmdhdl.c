#include <yatester/cmdhdl.h>
#include <yatester/builtins.h>
#include <yatester/yatester.h>

#include <string.h>
#include <stddef.h>
#include <stdio.h>

static const yatester_command** commandtable;
static size_t tablesize;

/**
 * @brief Hashes string into an unsigned long
 * @param str string to be hashed
 * @return hash
 */
static unsigned long hash_string_internal(const char* str)
{
	unsigned long hash = 5381;
	char c;

	while (c = *str++)
	{
		hash = ((hash << 5) + hash) + c;
	}

	return hash;
}

yatester_status yatester_initializecmdhdl()
{
	size_t i, j;
	const yatester_command* command;

	/* List of all commands added to the command table */
	const yatester_command* all_commands[] = 
	{
		yatester_builtin_commands,
		yatester_commands,
	};

	tablesize = 1; /* malloc(0) is undefined behaviour */

	/* Calculate the table size in terms of the desired load factor
	 * and the total number of commands (which does not change) */
	for (i = 0; i < sizeof(all_commands)/sizeof(*all_commands); ++i)
	{
		for (command = all_commands[i]; command->name != NULL; ++command)
		{
			tablesize += 2; /* Load factor of 0.5 */
		}
	}

	/* Allocate zero-initialized table */
	commandtable = calloc(tablesize, sizeof *commandtable);

	if (commandtable == NULL)
	{
		fprintf(stderr, "Could not allocate command table\n");
		return YATESTER_MEM;
	}

	/* Populate the table with all commands */
	for (i = 0; i < sizeof(all_commands)/sizeof(*all_commands); ++i)
	{
		for (command = all_commands[i]; command->name != NULL; ++command)
		{
			if (command->handler == NULL)
			{
				fprintf(stderr, "Command \"%s\" does not have a handler\n", command->name);
				return YATESTER_ERR;
			}

			if (command->name[0] == '\0')
			{
				fprintf(stderr, "Command with empty string as name\n");
				return YATESTER_ERR;
			}

			/* Hash command name to calculate table index */
			j = hash_string_internal(command->name) % tablesize;

			/* Open addressing with linear probing */
			while (commandtable[j] != NULL)
			{
				if (strcmp(commandtable[j]->name, command->name) == 0)
				{
					fprintf(stderr, "Found two commands named \"%s\"\n", command->name);
					return YATESTER_ERR;
				}

				/* Visit next entry (wrapping around) */
				j = (j + 1) % tablesize;
			}

			/* Save entry */
			commandtable[j] = command;
		}
	}
	
	return YATESTER_OK;
}

void yatester_itercommands(void (*callback)(const yatester_command*))
{
	size_t i;

	for (i = 0; i < tablesize; ++i)
	{
		if (commandtable[i] != NULL)
		{
			callback(commandtable[i]);
		}
	}
}

const yatester_command* yatester_getcommand(const char* commandname)
{
	size_t j;

	j = hash_string_internal(commandname) % tablesize;

	/* Since load factor < 1, it will eventually stumble upon a hole */
	while (commandtable[j] != NULL && strcmp(commandname, commandtable[j]->name) != 0)
	{
		j = (j + 1) % tablesize;
	}

	return commandtable[j];
}

void yatester_terminatecmdhdl()
{
	if (commandtable != NULL)
	{
		free(commandtable);
		commandtable = NULL;
	}
}
