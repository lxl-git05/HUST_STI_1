// Con_Task.h — 通用任务队列调度器
// 生命周期: Setup(一次) → Run(每帧) → IsExit?→true → 自动出队下一个
// 用法: Mode_X_Setup 中 Con_Task_Init + Enqueue, Mode_X_Loop 中 Con_Task_Loop
#ifndef __CON_TASK_H
#define __CON_TASK_H

#include "Queue.h"

// ==================== 日志开关 ====================
 #define CON_TASK_LOG  // ★ 取消注释以开启任务日志（Serial1 输出）

// ==================== 全局任务枚举（所有 Mode 共用） ====================
typedef enum {
    TASK_NONE = 0,
    TASK_WAIT_TIME,       // 等待指定毫秒: p[0]=ms
		TASK_Motor_Speed,			// 电机速度控制: p[0]=速度rpm, p[1]=持续时间ms
		TASK_Motor_Angle,			// 电机角度控制: p[0]=目标角度°, p[1]=容差°(默认20)
		// 比赛逻辑
		Task_Tar_XY,					// 装置去到目标x,y位置
		Task_Down,						// 装置下降
		Task_Back,						// 装置回位
		Task_Elec,						// 取/放棋子
		Task_Up	 ,						// 装置上升
		// ★ 枚举总数，必须放最后
    TASK_COUNT             
} Task_Type;

// ==================== 任务回调函数类型 ====================
typedef void (*Task_SetupFunc)(float params[4]);     // 进入任务时调用一次
typedef void (*Task_RunFunc)  (float params[4]);     // 每帧主循环调用，可为 NULL
typedef bool (*Task_ExitFunc) (float params[4]);     // 返回 true 则自动切换下一个任务
typedef void (*Task_TickFunc) (float params[4]);     // 20ms 中断调用，可为 NULL

// ==================== 任务执行记录（性能分析用） ====================
#define TASK_RECORD_MAX 64              // 最多记录条数
// #define CON_TASK_RECORD_CLEAR_ON_INIT  // ★ 取消注释：Con_Task_Init 时自动清空记录数组

typedef struct {
    int         task_index;     // 任务序号（第1条=1, 第2条=2 ...）
    Task_Type   task_type;      // 任务枚举值
    float       time_s;         // 本次执行耗时（秒）
} Task_Record_Typedef;

// ==================== 任务描述表（按 Task_Type 索引） ====================
typedef struct {
    Task_SetupFunc Setup;
    Task_RunFunc   Run;
    Task_ExitFunc  IsExit;
    Task_TickFunc  Tick;       // 20ms Tick 回调，可为 NULL
} Task_Descriptor_Typedef;

// ==================== API ====================

// 注册任务表 + 清空队列 + 终止当前任务（可重复调用，每次 Mode 切换时调用）
void Con_Task_Init(const Task_Descriptor_Typedef *table, int size);

// 便捷入队（4 个 float 参数），Setup/Loop 中均可调用
void Con_Task_Enqueue(int task_type, float p0, float p1, float p2, float p3);

// 清空队列 + 终止当前任务（紧急停止）
void Con_Task_Clear(void);

// 主循环调度（State Machine: Setup → Run → IsExit → 自动出队）
void Con_Task_Loop(void);

// 20ms ISR 分发（分发到当前活跃任务的 .Tick 回调，无任务时零开销）
void Con_Task_Tick(void);

// 是否有任务正在执行
bool Con_Task_IsBusy(void);

// 当前任务类型枚举值（-1 = 空闲）
int  Con_Task_CurrType(void);

// 队列中剩余任务数
int  Con_Task_Remaining(void);

// ==================== 任务记录（性能分析） ====================

// 任务执行记录数组（循环写入，供调试/分析读取）
extern Task_Record_Typedef Task_Records[];
extern int                  Task_Record_Count;

// 清空任务记录
void Con_Task_RecordClear(void);

#endif
