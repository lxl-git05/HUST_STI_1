// Queue.h
#ifndef __QUEUE_H
#define __QUEUE_H

#include "MySystem.h"
#include "Control.h"

// 队列大小
#define QUEUE_SIZE 50

// 队列数据类型
typedef Hanger_Status_Typedef QueueData_Typedef;	// 将void*替换成需要的数据结构即可

// 队列结构体
typedef struct
{
    QueueData_Typedef data[QUEUE_SIZE];

    int front;      // 队头
    int rear;       // 队尾
    int size;       // 当前元素数量

} Queue_Typedef;


// 初始化
void Queue_Init(Queue_Typedef *queue);

// 判空
bool Queue_IsEmpty(Queue_Typedef *queue);

// 判满
bool Queue_IsFull(Queue_Typedef *queue);

// 清空
void Queue_Clear(Queue_Typedef *queue);

// 入队
bool Queue_Enqueue(
    Queue_Typedef *queue,
    QueueData_Typedef value
);

// 出队
bool Queue_Dequeue(
    Queue_Typedef *queue,
    QueueData_Typedef *value
);

// 查看队头
QueueData_Typedef Queue_Peek(
    Queue_Typedef *queue,
    QueueData_Typedef *value
);

// 获取元素数量
int Queue_Size(Queue_Typedef *queue);

#endif
