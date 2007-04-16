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
