#ifndef __STEPPER_H
#define __STEPPER_H

#include "MySystem.h"
#include "Emm_V5.h"

// 串口驱动步进电机

// 接收数据的宏定义
#define CMD_LEN 255

// 命令队列大小
#define CMD_QUEUE_SIZE 16

// ===================== 命令类型枚举 =====================
typedef enum {
    CMD_NONE = 0,       // 空命令
    CMD_VEL,            // 速度模式       (参数: vel, acc)
    CMD_REL_POS,        // 相对位置       (参数: vel, acc, angle)
    CMD_ABS_POS,        // 绝对位置       (参数: vel, acc, angle)
    CMD_RESET_ZERO,     // 位置清零
    CMD_STOP,           // 立即停车       (最高优先级)
    CMD_QUERY_POS,      // 查询位置
    CMD_QUERY_VEL,     // 查询速度
} Stepper_CmdType;

// ===================== 命令结构体 =====================
typedef struct {
    Stepper_CmdType type;    // 命令类型
    int16_t vel;             // 速度
    uint8_t acc;             // 加速度
    float angle;             // 角度
} Stepper_Cmd;

// ===================== 命令队列 =====================
typedef struct {
    Stepper_Cmd items[CMD_QUEUE_SIZE];  // 队列数组
    volatile uint8_t head;              // 队首 (出队位置)
    volatile uint8_t tail;              // 队尾 (入队位置)
    volatile uint8_t count;              // 队列中命令数量
} Stepper_Queue;

// ===================== 步进电机结构体 =====================
typedef struct {
    // 通信
    UART_HandleTypeDef *Stepper_huart;  // 串口驱动口
    DMA_HandleTypeDef  *Stepper_dma;    // DMA驱动口
    uint8_t addr;                        // 地址
    uint8_t rxCount;                     // 读取的数据数
    bool rxFlag;                         // 1为接收
    uint8_t rx_buf[CMD_LEN];             // 接收缓冲
    volatile bool tx_busy;               // TX忙标志，发送时置1，TX完成中断清0

    // 命令队列
    Stepper_Queue cmd_queue;             // 命令队列

    // 电机参数
    float pulse_angle;                    // 单位脉冲角度(度)
    int8_t Positive_Dir;                // 正方向(1 or -1)

    // 速度
    int Speed_Tar;                       // 目标速度
    int Speed_Now;                       // 当前速度

    // 角度
    float Pos_Tar;                       // 目标角度
    float Pos_Now;                       // 当前角度

} Stepper_Typedef;

extern Stepper_Typedef Stepper1;  // 1号步进电机
extern Stepper_Typedef Stepper2;  // 2号步进电机
     
// ===================== 入队函数 (主循环/中断调用) =====================
// 速度模式入队
bool Stepper_Vel_Enqueue(Stepper_Typedef* pStepper, int16_t vel, uint8_t acc);
// 相对位置入队
bool Stepper_RelPos_Enqueue(Stepper_Typedef* pStepper, int16_t vel, uint8_t acc, float angle);
// 绝对位置入队
bool Stepper_AbsPos_Enqueue(Stepper_Typedef* pStepper, int16_t vel, uint8_t acc, float angle);
// 位置清零入队
bool Stepper_ResetZero_Enqueue(Stepper_Typedef* pStepper);
// 停车入队 (队列满也保证执行，丢弃最旧命令)
bool Stepper_Stop_Enqueue(Stepper_Typedef* pStepper);

// ===================== 轮询调度函数 (5ms定时器调用) =====================
// 5ms定时器轮询，处理队列命令和状态查询
void Stepper_Polling_5ms(void);

// ===================== 初始化 =====================
void Stepper_Init(void);

// ===================== 串口相关 =====================
// 串口RX_DMA处理,放在stm32h7xx_it.c的相应串口位置
void Stepper_Rx_DMA_Cplt(Stepper_Typedef* pStepper);

// ===================== 保留的旧API (兼容模式，已不推荐使用) =====================
// 这些函数内部调用会入队，但存在死锁风险，请优先使用 Enqueue 系列
void Stepper_Speed_Set(Stepper_Typedef* pStepper, int16_t vel, uint8_t acc);
void Stepper_Relative_Pos_Set(Stepper_Typedef* pStepper, int16_t vel, uint8_t acc, float angle);
void Stepper_Absolute_Pos_Set(Stepper_Typedef* pStepper, int16_t vel, uint8_t acc, float angle);
void Stepper_Reset_Zero(Stepper_Typedef* pStepper);

#endif
