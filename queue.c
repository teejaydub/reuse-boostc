// queue.c

#define IN_QUEUE

#include <system.h>

#include "queue.h"

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
