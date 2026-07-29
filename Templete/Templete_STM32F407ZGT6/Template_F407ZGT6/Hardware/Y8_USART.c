#include "Y8_USART.h"
#include "usart.h"
#include <stdio.h>

// ============== 全局数据 ==============
uint16_t Y8U_ADC[8] = {0};         // 8路模拟型 ADC
uint8_t  Y8U_Digital[8] = {0};     // 8路数字型

static volatile uint8_t new_data_flag = 0;   // 新帧标志（ISR 置位，主循环读取清零）

// ============== DMA 接收缓冲 ==============
static uint8_t y8u_rx_buf[Y8U_RX_BUF_SIZE];

// ============== 协议解析状态机 ==============
static char    parse_buf[64];       // 帧组装缓冲区
static uint8_t parse_idx = 0;       // 当前写入位置
static uint8_t parse_state = 0;     // 0=IDLE, 1=GOT_DOLLAR, 2=COLLECT

// ============== 内部函数声明 ==============
static void Y8U_FeedByte(uint8_t byte);
static void Y8U_ParseFrame(char *buf);

// ============== 初始化 ==============
void Y8U_Init(void)
{
    // 清空全局数据
    for (int i = 0; i < 8; i++) {
        Y8U_ADC[i] = 0;
        Y8U_Digital[i] = 0;
    }
    new_data_flag = 0;
    parse_state = 0;
    parse_idx = 0;

    // 启动 DMA+空闲中断接收（DMA 已由 CubeMX 在 HAL_UART_MspInit 中配置）
    HAL_UARTEx_ReceiveToIdle_DMA(&huart3, y8u_rx_buf, Y8U_RX_BUF_SIZE);

    // 上电后请求模拟型数据
    Y8U_SendCmd(0, 1, 0);
}

// ============== DMA 空闲中断回调（由 Serial_porting.c 调用）==============
void Y8U_DMA_RxCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    // 遍历 DMA 缓冲区中本次收到的数据，送入状态机
    for (uint16_t i = 0; i < Size; i++) {
        Y8U_FeedByte(y8u_rx_buf[i]);
    }

    // 重新启动 DMA 接收
    HAL_UARTEx_ReceiveToIdle_DMA(&huart3, y8u_rx_buf, Y8U_RX_BUF_SIZE);
}

// ============== 逐字节送入状态机 ==============
static void Y8U_FeedByte(uint8_t byte)
{
    switch (parse_state) {
    case 0:  // IDLE —— 等待 '$'
        if (byte == '$') {
            parse_buf[0] = '$';
            parse_idx = 1;
            parse_state = 1;
        }
        break;

    case 1:  // GOT_DOLLAR —— 等待 'A' 或 'D'
        parse_buf[parse_idx++] = (char)byte;
        parse_state = 2;
        break;

    case 2:  // COLLECT —— 收集数据直到 '#'
        if (parse_idx < sizeof(parse_buf) - 1) {
            parse_buf[parse_idx++] = (char)byte;
        }
        if (byte == '#') {
            parse_buf[parse_idx] = '\0';
            Y8U_ParseFrame(parse_buf);
            parse_state = 0;
        }
        break;
    }
}
#include "Timer_Counter.h"
// ============== 帧解析 ==============
static void Y8U_ParseFrame(char *buf)
{
    uint16_t val[8];
    int n;

    if (buf[1] == 'A') {
        // $A,x1:4096,x2:4096,...,x8:4096#
        n = sscanf(buf,
            "$A,x%*d:%hu,x%*d:%hu,x%*d:%hu,x%*d:%hu,"
             "x%*d:%hu,x%*d:%hu,x%*d:%hu,x%*d:%hu",
            &val[0], &val[1], &val[2], &val[3],
            &val[4], &val[5], &val[6], &val[7]);
        if (n == 8) {
            for (int i = 0; i < 8; i++) Y8U_ADC[i] = val[i];
            new_data_flag = 1;
//						Timer_Counter_Func() ;
        }
    }
    else if (buf[1] == 'D') {
        // $D,x1:0,x2:0,...,x8:0#
        n = sscanf(buf,
            "$D,x%*d:%hu,x%*d:%hu,x%*d:%hu,x%*d:%hu,"
             "x%*d:%hu,x%*d:%hu,x%*d:%hu,x%*d:%hu",
            &val[0], &val[1], &val[2], &val[3],
            &val[4], &val[5], &val[6], &val[7]);
        if (n == 8) {
            for (int i = 0; i < 8; i++) Y8U_Digital[i] = (uint8_t)val[i];
            new_data_flag = 1;
        }
    }
}

// ============== 命令发送 ==============
void Y8U_SendCmd(uint8_t calib, uint8_t analog, uint8_t digital)
{
    char cmd[16];
    int len = snprintf(cmd, sizeof(cmd), "$%d,%d,%d#", calib, analog, digital);
    if (len > 0 && len < (int)sizeof(cmd)) {
        HAL_UART_Transmit(&huart3, (uint8_t *)cmd, (uint16_t)len, 100);
    }
}

// ============== 新数据标志查询 ==============
uint8_t Y8U_NewData(void)
{
    if (new_data_flag) {
        new_data_flag = 0;
        return 1;
    }
    return 0;
}
