// Con_Task.c — 通用任务队列调度器实现
#include "Con_Task.h"
#include "AllHeader.h"
#include <string.h>

// ==================== 内部状态 ====================
static Queue_Typedef                   Task_Queue;        // 任务队列
static const Task_Descriptor_Typedef  *Task_Table;        // 任务描述表指针
static int                             Task_Table_Size;   // 表大小
static int                             Task_Curr = -1;    // 当前任务类型（-1=空闲）
static int                             Task_Next = -1;    // 下一个任务类型
static bool                            Task_SetupDone;    // 当前任务 Setup 是否已执行
static float                           Task_Params[4];    // 当前任务参数

// ==================== API 实现 ====================

// 注册任务表 + 清空队列 + 终止当前任务（可重复调用）
void Con_Task_Init(const Task_Descriptor_Typedef *table, int size)
{
    Task_Table      = table;
    Task_Table_Size = size;
    Queue_Clear(&Task_Queue);   // 清空队列（丢弃旧 Mode 残留任务）
    Task_Curr       = -1;       // 终止当前任务
    Task_Next       = -1;
    Task_SetupDone  = false;
    // Task_Params 下次出队时被 memcpy 覆盖，无需清零
}

// 便捷入队
void Con_Task_Enqueue(int task_type, float p0, float p1, float p2, float p3)
{
    QueueData_Typedef entry;
    entry.type      = task_type;
    entry.params[0] = p0;
    entry.params[1] = p1;
    entry.params[2] = p2;
    entry.params[3] = p3;
    Queue_Enqueue(&Task_Queue, entry);
}

// 清空队列 + 终止当前任务
void Con_Task_Clear(void)
{
    Queue_Clear(&Task_Queue);
    Task_Curr      = -1;
    Task_Next      = -1;
    Task_SetupDone = false;
}

// 主循环调度
void Con_Task_Loop(void)
{
    // 1. 空闲且队列不空 → 出队下一个任务
    if (Task_Curr == -1 && !Queue_IsEmpty(&Task_Queue))
    {
        QueueData_Typedef entry;
        Queue_Dequeue(&Task_Queue, &entry);
        Task_Next = entry.type;

        // ★ 边界检查：越界则 LED 快闪 + 死循环
        if (Task_Next < 0 || Task_Next >= Task_Table_Size)
        {
            Flash_Mode_Set(Flash_Mode_Fast);
            while (1);
        }

        // params 覆盖旧任务参数
        memcpy(Task_Params, entry.params, sizeof(Task_Params));
    }

    // 2. 状态切换: curr != next → 执行新任务 Setup
    if (Task_Curr != Task_Next && Task_Next != -1)
    {
        const Task_Descriptor_Typedef *desc = &Task_Table[Task_Next];
        if (desc->Setup) desc->Setup(Task_Params);
        Task_SetupDone = true;
        Task_Curr = Task_Next;
    }

    // 3. 当前任务 Run
    if (Task_Curr != -1 && Task_SetupDone)
    {
        const Task_Descriptor_Typedef *desc = &Task_Table[Task_Curr];
        if (desc->Run) desc->Run(Task_Params);
    }

    // 4. 检查退出条件
    if (Task_Curr != -1 && Task_SetupDone)
    {
        const Task_Descriptor_Typedef *desc = &Task_Table[Task_Curr];
        if (desc->IsExit && desc->IsExit(Task_Params))
        {
            Task_Curr = -1;
            Task_Next = -1;
            Task_SetupDone = false;
            // 下个 Loop 周期自动回到步骤1，出队下一个任务
        }
    }
}

// 20ms ISR 分发
void Con_Task_Tick(void)
{
    if (Task_Curr != -1 && Task_SetupDone)
    {
        const Task_Descriptor_Typedef *desc = &Task_Table[Task_Curr];
        if (desc->Tick) desc->Tick(Task_Params);
    }
}

// 是否有任务正在执行
bool Con_Task_IsBusy(void)
{
    return (Task_Curr != -1);
}

// 当前任务类型
int Con_Task_CurrType(void)
{
    return Task_Curr;
}

// 队列剩余任务数
int Con_Task_Remaining(void)
{
    return Queue_Size(&Task_Queue);
}
