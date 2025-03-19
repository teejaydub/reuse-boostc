/* queue.c
    Copyright (c) 2007, 2009 by Timothy J. Weber, tw@timothyweber.org.
*/

#define IN_QUEUE

#include <system.h>

#include "queue.h"

QueueEntry* QueueIncrement(QueueEntry* queueIndex)
{
	if (queueIndex == &queue[QUEUE_LENGTH - 1])
		return &queue;
	else
		return ++queueIndex;
}

void PrePushQueue(void)
{
	if (IsQueueFull()) {
		PopQueue();
	}
}

void PrePushQueueKeepHead(void)
{
	if (IsQueueFull()) {
		queueTail = QueueNextHead();
		queueCount = 1;
	}
}

void ClearQueueTail(void)
{
	if (!IsQueueEmpty()) {
		queueTail = QueueNextHead();
		queueCount = 1;
	}
}
