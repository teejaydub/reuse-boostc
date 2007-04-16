// queue.c

#define IN_QUEUE

#include <system.h>

#include "queue.h"

byte QueueIncrement(byte queueIndex)
{
	if (queueIndex == QUEUE_LENGTH - 1)
		return 0;
	else
		return queueIndex + 1;
}

void PrePushQueue(void)
{
	if (IsQueueFull())
		queueHead = QueueNextHead();
}

void PrePushQueueKeepHead(void)
{
	if (IsQueueFull())
		queueTail = QueueNextHead();
}

void ClearQueueTail(void)
{
	if (!IsQueueEmpty())
		queueTail = QueueNextHead();
}
