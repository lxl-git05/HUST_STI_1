#include "Stepper.h"

Stepper_Typedef Stepper1;  // 1号步进电机
Stepper_Typedef Stepper2;  // 2号步进电机

#define Stepper1_huart huart3
#define Stepper2_huart huart6
#define Stepper1_hdma_usart_rx hdma_usart3_rx
#define Stepper2_hdma_usart_rx hdma_usart6_rx

extern DMA_HandleTypeDef Stepper1_hdma_usart_rx;
extern DMA_HandleTypeDef Stepper2_hdma_usart_rx;

// ===================== 队列操作(原子化) =====================

// 队列是否为空
static bool Queue_IsEmpty(Stepper_Queue* q)
{
    return q->count == 0;
}

// 队列是否满
static bool Queue_IsFull(Stepper_Queue* q)
{
    return q->count >= CMD_QUEUE_SIZE;
}

// 入队 - 线程安全版本 (可中断调用)
static bool Queue_Push_ISR(Stepper_Queue* q, Stepper_Cmd* cmd)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    if (Queue_IsFull(q)) {
        __set_PRIMASK(primask);
        return false;
    }

    q->items[q->tail] = *cmd;
    q->tail = (q->tail + 1) % CMD_QUEUE_SIZE;
    q->count++;

    __set_PRIMASK(primask);
    return true;
}

// 入队 - 强制版本 (丢弃最旧命令保证入队，用于STOP)
static bool Queue_Push_Force(Stepper_Queue* q, Stepper_Cmd* cmd)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    if (Queue_IsFull(q)) {
        // 队列满，丢弃最旧的命令
        q->head = (q->head + 1) % CMD_QUEUE_SIZE;
        q->count--;
    }

    q->items[q->tail] = *cmd;
    q->tail = (q->tail + 1) % CMD_QUEUE_SIZE;
    q->count++;

    __set_PRIMASK(primask);
    return true;
}

// 出队 - 线程安全版本 (5ms定时器调用)
static bool Queue_Pop_ISR(Stepper_Queue* q, Stepper_Cmd* cmd)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    if (Queue_IsEmpty(q)) {
        __set_PRIMASK(primask);
        return false;
    }

    *cmd = q->items[q->head];
    q->head = (q->head + 1) % CMD_QUEUE_SIZE;
    q->count--;

    __set_PRIMASK(primask);
    return true;
}

// ===================== 命令执行 =====================

static void Stepper_Exec_Cmd(Stepper_Typedef* pStepper, Stepper_Cmd* cmd)
{
    if (pStepper->tx_busy) return;
    pStepper->tx_busy = true;

    switch (cmd->type) {
        case CMD_VEL:
            if (cmd->vel * pStepper->Positive_Dir > 0) {
                Emm_V5_Vel_Control(pStepper->Stepper_huart, pStepper->addr, 0, cmd->vel, cmd->acc, false);
            } else {
                Emm_V5_Vel_Control(pStepper->Stepper_huart, pStepper->addr, 1, -cmd->vel, cmd->acc, false);
            }
            break;

        case CMD_REL_POS: {
            int16_t vel = (cmd->vel > 0) ? cmd->vel : -cmd->vel;
            uint8_t dir = (cmd->vel * pStepper->Positive_Dir > 0) ? 0 : 1;
            uint32_t clk = (uint32_t)(cmd->angle / pStepper->pulse_angle);
            Emm_V5_Pos_Control(pStepper->Stepper_huart, pStepper->addr, dir, vel, cmd->acc, clk, 2, false);
            break;
        }

        case CMD_ABS_POS: {
            int16_t vel = (cmd->vel > 0) ? cmd->vel : -cmd->vel;
            uint8_t dir = (cmd->vel * pStepper->Positive_Dir > 0) ? 0 : 1;
            uint32_t clk = (uint32_t)(cmd->angle / pStepper->pulse_angle);
            Emm_V5_Pos_Control(pStepper->Stepper_huart, pStepper->addr, dir, vel, cmd->acc, clk, 1, false);
            break;
        }

        case CMD_STOP:
            Emm_V5_Stop_Now(pStepper->Stepper_huart, pStepper->addr, false);
            break;

        case CMD_RESET_ZERO:
            Emm_V5_Reset_CurPos_To_Zero(pStepper->Stepper_huart, pStepper->addr);
            break;

        case CMD_QUERY_POS:
            Emm_V5_Read_Sys_Params(pStepper->Stepper_huart, pStepper->addr, S_CPOS);
            break;

        case CMD_QUERY_VEL:
            Emm_V5_Read_Sys_Params(pStepper->Stepper_huart, pStepper->addr, S_VEL);
            break;

        default:
            pStepper->tx_busy = false;
            break;
    }
}

// ===================== 入队API (主循环/中断调用) =====================

bool Stepper_Vel_Enqueue(Stepper_Typedef* pStepper, int16_t vel, uint8_t acc)
{
    Stepper_Cmd cmd = { CMD_VEL, vel, acc, 0 };
    return Queue_Push_ISR(&pStepper->cmd_queue, &cmd);
}

bool Stepper_RelPos_Enqueue(Stepper_Typedef* pStepper, int16_t vel, uint8_t acc, float angle)
{
    Stepper_Cmd cmd = { CMD_REL_POS, vel, acc, angle };
    return Queue_Push_ISR(&pStepper->cmd_queue, &cmd);
}

bool Stepper_AbsPos_Enqueue(Stepper_Typedef* pStepper, int16_t vel, uint8_t acc, float angle)
{
    Stepper_Cmd cmd = { CMD_ABS_POS, vel, acc, angle };
    return Queue_Push_ISR(&pStepper->cmd_queue, &cmd);
}

bool Stepper_ResetZero_Enqueue(Stepper_Typedef* pStepper)
{
    Stepper_Cmd cmd = { CMD_RESET_ZERO, 0, 0, 0 };
    return Queue_Push_ISR(&pStepper->cmd_queue, &cmd);
}

// 停车命令 - 队列满也保证执行(丢弃最旧命令)
bool Stepper_Stop_Enqueue(Stepper_Typedef* pStepper)
{
    Stepper_Cmd cmd = { CMD_STOP, 0, 0, 0 };
    return Queue_Push_Force(&pStepper->cmd_queue, &cmd);
}

// ===================== 5ms定时器轮询 =====================

typedef enum {
    QUERY_POS = 0,
    QUERY_VEL = 1
} Query_State;

static Query_State Motor1_QueryState = QUERY_POS;
static Query_State Motor2_QueryState = QUERY_POS;

void Stepper_Polling_5ms(void)
{
    Stepper_Cmd cmd;

    // ========== 电机1 ==========
    if (!Stepper1.tx_busy) {
        if (Queue_Pop_ISR(&Stepper1.cmd_queue, &cmd)) {
            // 队列有命令 -> 执行命令
            Stepper_Exec_Cmd(&Stepper1, &cmd);
        } else {
            // 队列空 -> 查询状态
            if (Motor1_QueryState == QUERY_POS) {
                Motor1_QueryState = QUERY_VEL;
                cmd.type = CMD_QUERY_POS;
            } else {
                Motor1_QueryState = QUERY_POS;
                cmd.type = CMD_QUERY_VEL;
            }
            cmd.vel = 0;
            cmd.acc = 0;
            cmd.angle = 0;
            Stepper_Exec_Cmd(&Stepper1, &cmd);
        }
    }

    // ========== 电机2 (独立，不受电机1影响) ==========
    if (!Stepper2.tx_busy) {
        if (Queue_Pop_ISR(&Stepper2.cmd_queue, &cmd)) {
            Stepper_Exec_Cmd(&Stepper2, &cmd);
        } else {
            if (Motor2_QueryState == QUERY_POS) {
                Motor2_QueryState = QUERY_VEL;
                cmd.type = CMD_QUERY_POS;
            } else {
                Motor2_QueryState = QUERY_POS;
                cmd.type = CMD_QUERY_VEL;
            }
            cmd.vel = 0;
            cmd.acc = 0;
            cmd.angle = 0;
            Stepper_Exec_Cmd(&Stepper2, &cmd);
        }
    }
}

// ===================== 初始化 =====================

void Stepper_Init(void)
{
    // 关闭DMA的缓存
    SCB_DisableDCache();  // 一定要记得这个，否则缓存更新会使得数组数据不更新

    // 电机1
    Stepper1.Stepper_huart = &Stepper1_huart;
    Stepper1.Stepper_dma   = &Stepper1_hdma_usart_rx;
    Stepper1.addr = 0x1;
    Stepper1.Positive_Dir = 1;
    Stepper1.pulse_angle = 0.004f;  // 1.8f / (75.0f / 27.0f) / 162
    Stepper1.tx_busy = false;
    Stepper1.cmd_queue.head = 0;
    Stepper1.cmd_queue.tail = 0;
    Stepper1.cmd_queue.count = 0;

    // 电机2
    Stepper2.Stepper_huart = &Stepper2_huart;
    Stepper2.Stepper_dma   = &Stepper2_hdma_usart_rx;
    Stepper2.addr = 0x1;
    Stepper2.Positive_Dir = 1;
    Stepper2.pulse_angle = 0.01f;  // 1.8f / 180
    Stepper2.tx_busy = false;
    Stepper2.cmd_queue.head = 0;
    Stepper2.cmd_queue.tail = 0;
    Stepper2.cmd_queue.count = 0;

    // RX_DMA
    HAL_UARTEx_ReceiveToIdle_DMA(&Stepper1_huart, (uint8_t *)Stepper1.rx_buf, CMD_LEN);
    HAL_UARTEx_ReceiveToIdle_DMA(&Stepper2_huart, (uint8_t *)Stepper2.rx_buf, CMD_LEN);

    // 等待初始化完成
    HAL_Delay(500);
}

// ===================== 保留的旧API (兼容模式，已不推荐使用) =====================
// 注意：这些函数内部使用while等待，存在死锁风险，请优先使用 Enqueue 系列

void Stepper_Speed_Set(Stepper_Typedef* pStepper, int16_t vel, uint8_t acc)
{
    while (pStepper->tx_busy);  // 等待TX完成
    pStepper->tx_busy = true;

    if (vel * pStepper->Positive_Dir > 0) {
        Emm_V5_Vel_Control(pStepper->Stepper_huart, pStepper->addr, 0, vel, acc, false);
    } else {
        Emm_V5_Vel_Control(pStepper->Stepper_huart, pStepper->addr, 1, -vel, acc, false);
    }
}

void Stepper_Relative_Pos_Set(Stepper_Typedef* pStepper, int16_t vel, uint8_t acc, float angle)
{
    while (pStepper->tx_busy);
    pStepper->tx_busy = true;

    uint32_t clk = (uint32_t)(angle / pStepper->pulse_angle);
    uint8_t dir = (vel * pStepper->Positive_Dir > 0) ? 0 : 1;
    vel = abs(vel);
    Emm_V5_Pos_Control(pStepper->Stepper_huart, pStepper->addr, dir, vel, acc, clk, 2, false);
}

void Stepper_Absolute_Pos_Set(Stepper_Typedef* pStepper, int16_t vel, uint8_t acc, float angle)
{
    while (pStepper->tx_busy);
    pStepper->tx_busy = true;

    uint32_t clk = (uint32_t)(angle / pStepper->pulse_angle);
    uint8_t dir = (vel * pStepper->Positive_Dir > 0) ? 0 : 1;
    vel = abs(vel);
    Emm_V5_Pos_Control(pStepper->Stepper_huart, pStepper->addr, dir, vel, acc, clk, 1, false);
}

void Stepper_Reset_Zero(Stepper_Typedef* pStepper)
{
    while (pStepper->tx_busy);
    pStepper->tx_busy = true;

    Emm_V5_Reset_CurPos_To_Zero(pStepper->Stepper_huart, pStepper->addr);
}

// ===================== 相应回复信息 =====================

// 实时速度接收处理
void Stepper_Speed_Deal(Stepper_Typedef* pStepper)
{
    if (pStepper->rx_buf[0] == pStepper->addr && pStepper->rx_buf[1] == 0x35 && pStepper->rxCount == 6) {
        // 拼接成uint16_t类型数据
        pStepper->Speed_Now = (uint16_t)(
            ((uint16_t)pStepper->rx_buf[3] << 8) |
            ((uint16_t)pStepper->rx_buf[4] << 0)
        );

        // 符号
        if (pStepper->rx_buf[2]) {
            pStepper->Speed_Now = -pStepper->Speed_Now;
        }
    }
}

// 实时角度接收处理
void Stepper_Angle_Deal(Stepper_Typedef* pStepper)
{
    if (pStepper->rx_buf[0] == pStepper->addr && pStepper->rx_buf[1] == 0x36 && pStepper->rxCount == 8) {
        // 拼接成uint32_t类型
        float pos = (uint32_t)(
            ((uint32_t)pStepper->rx_buf[3] << 24) |
            ((uint32_t)pStepper->rx_buf[4] << 16) |
            ((uint32_t)pStepper->rx_buf[5] << 8)  |
            ((uint32_t)pStepper->rx_buf[6] << 0)
        );

        // 转换成角度
        pStepper->Pos_Now = (float)pos * 360.0f / 65536.0f;

        // 符号
        if (pStepper->rx_buf[2]) {
            pStepper->Pos_Now = -pStepper->Pos_Now;
        }
    }
}

// 串口RX_DMA处理,放在stm32h7xx_it.c的相应串口位置
void Stepper_Rx_DMA_Cplt(Stepper_Typedef* pStepper)
{
    if (__HAL_UART_GET_FLAG(pStepper->Stepper_huart, UART_FLAG_IDLE) != RESET) {
        __HAL_UART_CLEAR_IDLEFLAG(pStepper->Stepper_huart);

        HAL_UART_DMAStop(pStepper->Stepper_huart);

        // 数据处理
        pStepper->rxCount = CMD_LEN - __HAL_DMA_GET_COUNTER(pStepper->Stepper_dma);

        pStepper->rxFlag = true;

        // 接收数据特殊处理
        Stepper_Speed_Deal(pStepper);
        Stepper_Angle_Deal(pStepper);

        // 重启
        HAL_UARTEx_ReceiveToIdle_DMA(pStepper->Stepper_huart, (uint8_t *)pStepper->rx_buf, CMD_LEN);
    }
}
