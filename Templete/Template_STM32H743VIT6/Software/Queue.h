// Queue.h — 环形队列（循环缓冲区），通用数据类型
#ifndef __QUEUE_H
#define __QUEUE_H

#include "MySystem.h"

// 队列大小
#define QUEUE_SIZE 50

// ==================== 队列数据类型 ====================
// Con_Task 任务队列专用: 任务类型枚举 + 最多 4 个 float 参数
typedef struct {
    int   type;          // 任务类型枚举值 (Task_Type)
    float params[4];     // 通用参数槽
} QueueData_Typedef;

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

// 查看队头（不出队）
bool Queue_Peek(
    Queue_Typedef *queue,
    QueueData_Typedef *value
);

// 获取元素数量
int Queue_Size(Queue_Typedef *queue);

#endif
