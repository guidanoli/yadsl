#include "queue.h"

#include "posixstring.h"
#include <stdio.h>
#include <stdlib.h>

#include "tester.h"
#include "testerutils.h"

const char *TesterHelpStrings[] = {
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
static Queue *pQueue = NULL;
static TesterReturnValue convertReturnValue(QueueRet queueId);

TesterReturnValue TesterInitCallback()
{
	if (queueCreate(&pQueue, free))
		return TESTER_MALLOC;
	return TESTER_OK;
}

TesterReturnValue TesterParseCallback(const char *command)
{
	char *str1, *str2;
	QueueRet queueId = QUEUE_OK;
	if matches(command, "queue") {
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_ARGUMENT;
		if ((str1 = strdup(buffer)) == NULL)
			return TESTER_MALLOC;
		if (queueId = queueQueue(pQueue, str1))
			free(str1);
	} else if matches(command, "dequeue") {
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_ARGUMENT;
		if ((str1 = strdup(buffer)) == NULL)
			return TESTER_MALLOC;
		if (queueId = queueDequeue(pQueue, &str2)) {
			free(str1);
		} else {
			int equal = matches(str1, str2);
			free(str1);
			free(str2);
			if (!equal)
				return TESTER_RETURN;
		}
	} else if matches(command, "empty") {
		int expected, obtained;
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_ARGUMENT;
		expected = TesterGetYesOrNoFromString(buffer);
		queueId = queueIsEmpty(pQueue, &obtained);
		if (queueId == QUEUE_OK && expected != obtained)
			return TESTER_RETURN;
	} else {
		return TESTER_COMMAND;
	}
	return convertReturnValue(queueId);
}

TesterReturnValue TesterExitCallback()
{
	queueDestroy(pQueue);
	pQueue = NULL;
	return TESTER_OK;
}

static TesterReturnValue convertReturnValue(QueueRet queueId)
{
	switch (queueId) {
	case QUEUE_OK:
		return TESTER_OK;
	case QUEUE_EMPTY:
		return TesterExternalReturnValue("empty");
	case QUEUE_MEMORY:
		return TesterExternalReturnValue("malloc");
	default:
		return TesterExternalReturnValue("unknown");
	}
}
