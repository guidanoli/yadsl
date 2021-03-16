#include <yatester/errhdl.h>

#include <stdio.h>

static yatester_status expected_status[2]; /* Queue */

void yatester_pushexpectedstatus(yatester_status status)
{
	expected_status[1] = status; /* Push to queue */
}

yatester_status yatester_getexpectedstatus()
{
	return expected_status[0];
}

void yatester_popexpectedstatus()
{
	/* Pop from queue */
	expected_status[0] = expected_status[1];
	expected_status[1] = YATESTER_OK;
}

yatester_status yatester_evaluatestatus(yatester_status status)
{
	if (status == yatester_getexpectedstatus())
	{
		if (status != YATESTER_OK)
		{
			fprintf(stderr, "Previous error was expected\n");
		}

		yatester_popexpectedstatus();

		return YATESTER_OK;
	}
	else
	{
		if (status == YATESTER_OK)
		{
			return YATESTER_ERR;
		}
		else
		{
			return status;
		}
	}
}
