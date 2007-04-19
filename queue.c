// queue.c

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
		queueHead = QueueNextHead();
		--queueCount;
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
