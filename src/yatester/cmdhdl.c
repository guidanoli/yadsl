#include <yatester/cmdhdl.h>
#include <yatester/builtins.h>
#include <yatester/yatester.h>

#include <string.h>
#include <stddef.h>
#include <stdio.h>

static const yatester_command** commandtable;
static size_t tablesize;

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
	const yatester_command* all_commands[] = 
	{
		yatester_builtin_commands,
		yatester_commands,
	};

	tablesize = 0;

	for (size_t i = 0; i < sizeof(all_commands)/sizeof(*all_commands); ++i)
	{
		const yatester_command* commands = all_commands[i];

		while (commands->name != NULL)
		{
			commands++;
			tablesize += 2; /* Load factor of 0.5 */
		}
	}

	commandtable = calloc(tablesize, sizeof *commandtable);

	if (commandtable == NULL)
	{
		fprintf(stderr, "Could not allocate command table\n");
		return YATESTER_MEM;
	}

	for (size_t i = 0; i < sizeof(all_commands)/sizeof(*all_commands); ++i)
	{
		const yatester_command* commands = all_commands[i];

		while (commands->name != NULL)
		{
			size_t j;

			if (commands->argc < 0)
			{
				fprintf(stderr, "Command \"%s\" requires negative number of arguments\n", commands->name);
				return YATESTER_ERR;
			}

			if (commands->handler == NULL)
			{
				fprintf(stderr, "Command \"%s\" does not have a handler\n", commands->name);
				return YATESTER_ERR;
			}

			j = hash_string_internal(commands->name) % tablesize;

			/* Open addressing with linear probing */
			while (commandtable[j] != NULL)
			{
				if (strcmp(commandtable[j]->name, commands->name) == 0)
				{
					fprintf(stderr, "Found two commands named \"%s\"\n", commands->name);
					return YATESTER_ERR;
				}

				j = (j + 1) % tablesize;
			}

			commandtable[j] = commands;
			++commands;
		}
	}
	
	return YATESTER_OK;
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
