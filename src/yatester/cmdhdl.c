#include <yatester/cmdhdl.h>
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
	size_t i, j, ncommands;
	const yatester_command* command;

	for (ncommands = 0; yatester_commands[ncommands].name != NULL ; ++ncommands);

	tablesize = 2 * ncommands; /* Load factor of 0.5 */
	commandtable = calloc(tablesize, sizeof(yatester_command*));

	if (commandtable == NULL)
	{
		fprintf(stderr, "Could not allocate command table\n");
		return YATESTER_MEM;
	}

	for (i = 0; i < ncommands; ++i)
	{
		command = &yatester_commands[i];
		j = hash_string_internal(command->name) % tablesize;

		/* Open addressing with linear probing */
		while (commandtable[j] != NULL)
		{
			if (strcmp(commandtable[j]->name, command->name) == 0)
			{
				fprintf(stderr, "Found two commands named \"%s\"\n", command->name);
				return YATESTER_ERR;
			}

			j = (j + 1) % tablesize;
		}

		commandtable[j] = command;
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
