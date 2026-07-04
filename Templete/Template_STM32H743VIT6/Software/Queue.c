#include "Queue.h"

// 初始化
void Queue_Init(Queue_Typedef *queue)
{
    queue->front = 0;
    queue->rear  = 0;
    queue->size  = 0;
}

// 判空
bool Queue_IsEmpty(Queue_Typedef *queue)
{
    return (queue->size == 0);
}

// 判满
bool Queue_IsFull(Queue_Typedef *queue)
{
    return (queue->size >= QUEUE_SIZE);
}

// 清空
void Queue_Clear(Queue_Typedef *queue)
{
    queue->front = 0;
    queue->rear  = 0;
    queue->size  = 0;
}

// 入队
bool Queue_Enqueue(
    Queue_Typedef *queue,
    QueueData_Typedef value
)
{
    if (Queue_IsFull(queue))
    {
        return false;
    }

    queue->data[queue->rear] = value;

    queue->rear++;

    if (queue->rear >= QUEUE_SIZE)
    {
        queue->rear = 0;
    }

    queue->size++;

    return true;
}

// 出队
bool Queue_Dequeue(
    Queue_Typedef *queue,
    QueueData_Typedef *value
)
{
    if (Queue_IsEmpty(queue))
    {
        return false;
    }

    if (value != NULL)
    {
        *value = queue->data[queue->front];
    }

    queue->front++;

    if (queue->front >= QUEUE_SIZE)
    {
        queue->front = 0;
    }

    queue->size--;

    return true;
}

// 查看队头
QueueData_Typedef Queue_Peek(
    Queue_Typedef *queue,
    QueueData_Typedef *value
)
{
    if (Queue_IsEmpty(queue))
    {
        return NULL;
    }

    if (value != NULL)
    {
        *value = queue->data[queue->front];
    }

    return queue->data[queue->front];
}

// 获取元素数量
int Queue_Size(Queue_Typedef *queue)
{
    return queue->size;
}

