// Con_Task.c — 通用任务队列调度器实现
#include "Con_Task.h"
#include "AllHeader.h"
#include <string.h>

// ==================== 内部状态 ====================
Queue_Typedef                   Task_Queue;        // 任务队列
const Task_Descriptor_Typedef  *Task_Table;        // 任务描述表指针
int                             Task_Table_Size;   // 表大小
int                             Task_Curr = -1;    // 当前任务类型（-1=空闲）
int                             Task_Next = -1;    // 下一个任务类型
bool                            Task_SetupDone;    // 当前任务 Setup 是否已执行
float                           Task_Params[4];    // 当前任务参数
uint32_t                        Task_StartTick;    // 当前任务启动时刻(ms)

// 任务执行记录
Task_Record_Typedef Task_Records[TASK_RECORD_MAX];
int                 Task_Record_Count = 0;
static int          Task_Exec_Index   = 0;   // 全局任务执行序号（每完成一个任务+1）

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

#ifdef CON_TASK_RECORD_CLEAR_ON_INIT
    Con_Task_RecordClear();     // 切 Mode 时自动清空记录，防止溢出
#endif
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

        Task_StartTick = HAL_GetTick();                         // ★ 记录任务开始时刻

#ifdef CON_TASK_LOG
        Serial_printf(&Serial1, "[Task:%d Setup]\r\n", Task_Next);
#endif

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
            // ★ 记录本次任务耗时
            float elapsed_s = (HAL_GetTick() - Task_StartTick) / 1000.0f;
            if (Task_Record_Count < TASK_RECORD_MAX)
            {
                Task_Exec_Index++;
                Task_Records[Task_Record_Count].task_index = Task_Exec_Index;
                Task_Records[Task_Record_Count].task_type  = (Task_Type)Task_Curr;
                Task_Records[Task_Record_Count].time_s     = elapsed_s;
                Task_Record_Count++;
            }

#ifdef CON_TASK_LOG
            Serial_printf(&Serial1, "[Task:%d Exit Time:%.2fs]\r\n", Task_Curr, elapsed_s);
#endif

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

// 清空任务记录
void Con_Task_RecordClear(void)
{
    Task_Record_Count = 0;
    Task_Exec_Index   = 0;
}
