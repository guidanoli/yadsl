#include <queue/queue.h>

#include <yadsl/posixstring.h>
#include <stdio.h>
#include <stdlib.h>

#include <tester/tester.h>
#include <testerutils/testerutils.h>

const char *yadsl_tester_help_strings[] = {
	"This is the queue test module",
	"You will be interacting with the same queue at all times",
	"The available actions are the following:",
	"",
	"/queue X     queue variable X",
	"/dequeue X   dequeue and compare top with X",
	"/empty <isempty> check whether queue is empty (<isempty> = true)",
	NULL
};

static char buffer[BUFSIZ];
static yadsl_QueueHandle *pQueue = NULL;
static yadsl_TesterRet convertReturnValue(yadsl_QueueRet queueId);

yadsl_TesterRet yadsl_tester_init()
{
	if (pQueue = yadsl_queue_create(free))
		return YADSL_TESTER_RET_OK;
	else
		return YADSL_TESTER_RET_MALLOC;
}

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
	char *str1, *str2;
	yadsl_QueueRet queueId = YADSL_QUEUE_RET_OK;
	if matches(command, "queue") {
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if ((str1 = strdup(buffer)) == NULL)
			return YADSL_TESTER_RET_MALLOC;
		if (queueId = yadsl_queue_queue(pQueue, str1))
			free(str1);
	} else if matches(command, "dequeue") {
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if ((str1 = strdup(buffer)) == NULL)
			return YADSL_TESTER_RET_MALLOC;
		if (queueId = yadsl_queue_dequeue(pQueue, &str2)) {
			free(str1);
		} else {
			int equal = matches(str1, str2);
			free(str1);
			free(str2);
			if (!equal)
				return YADSL_TESTER_RET_RETURN;
		}
	} else if matches(command, "empty") {
		bool expected, obtained;
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		expected = TesterUtilsGetYesOrNoFromString(buffer);
		queueId = yadsl_queue_empty_check(pQueue, &obtained);
		if (queueId == YADSL_QUEUE_RET_OK && expected != obtained)
			return YADSL_TESTER_RET_RETURN;
	} else {
		return YADSL_TESTER_RET_COMMAND;
	}
	return convertReturnValue(queueId);
}

yadsl_TesterRet yadsl_tester_release()
{
	yadsl_queue_destroy(pQueue);
	pQueue = NULL;
	return YADSL_TESTER_RET_OK;
}

static yadsl_TesterRet convertReturnValue(yadsl_QueueRet queueId)
{
	switch (queueId) {
	case YADSL_QUEUE_RET_OK:
		return YADSL_TESTER_RET_OK;
	case YADSL_QUEUE_RET_EMPTY:
		return yadsl_tester_return_external_value("empty");
	case YADSL_QUEUE_RET_MEMORY:
		return yadsl_tester_return_external_value("malloc");
	default:
		return yadsl_tester_return_external_value("unknown");
	}
}
