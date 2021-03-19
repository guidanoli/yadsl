#include <yatester/cmdhdl.h>
#include <yatester/builtins.h>
#include <yatester/yatester.h>

#include <math.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>

static const yatester_command** commandtable;
static size_t tablesize;

/**
 * @brief djb2 hash function
 */
static size_t djb2_hash(const char* str)
{
	size_t hash = 5381;
	char c;

	while (c = *str++)
	{
		hash = ((hash << 5) + hash) + c;
	}

	return hash;
}

/**
 * @brief Check if n is prime
 */
static int is_prime(size_t n)
{
	if (n < 2)
	{
		return 0;
	}
	else if (n == 2)
	{
		return 1;
	}
	else /* n > 2 */
	{
		for (size_t i = 3; i * i <= n; i += 2)
		{
			if (n % i == 0)
			{
				return 0;
			}
		}
		return 1;
	}
}

/**
 * @brief Get next prime after n
 */
static size_t next_prime(size_t n)
{
	while (!is_prime(++n));

	return n;
}

yatester_status yatester_initializecmdhdl()
{
	size_t i, j;
	const yatester_command* command;

	/* List of all commands to be added to the command table */
	const yatester_command* all_commands[] = 
	{
		yatester_builtin_commands,
		yatester_commands,
	};

	tablesize = 0;

	/* Count the number of commands */
	for (i = 0; i < sizeof(all_commands)/sizeof(*all_commands); ++i)
	{
		for (command = all_commands[i]; command->name != NULL; ++command)
		{
			tablesize += 4; /* Load factor < 1/4 */
		}
	}

	tablesize = next_prime(tablesize);

	/* Allocate zero-initialized table */
	commandtable = calloc(tablesize, sizeof *commandtable);

	if (commandtable == NULL)
	{
		return yatester_report(YATESTER_NOMEM, "could not allocate command table\n");
	}

	/* Populate the table with all commands */
	for (i = 0; i < sizeof(all_commands)/sizeof(*all_commands); ++i)
	{
		for (command = all_commands[i]; command->name != NULL; ++command)
		{
			if (command->handler == NULL)
			{
				return yatester_report(YATESTER_BADCMD, "command \"%s\" does not have a handler\n", command->name);
			}

			/* Hash command name to calculate table index */
			j = djb2_hash(command->name) % tablesize;

			/* Open addressing with linear probing */
			/* Since load factor < 1, it will eventually stumble upon a hole */
			while (commandtable[j] != NULL)
			{
				if (strcmp(commandtable[j]->name, command->name) == 0)
				{
					return yatester_report(YATESTER_BADCMD, "command \"%s\" already exists\n", command->name);
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

	j = djb2_hash(commandname) % tablesize;

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
