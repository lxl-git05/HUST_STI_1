#include "Serial_porting.h"
#include "string.h"
#include <stdarg.h>
#include <stdio.h>
#include "Timer_Counter.h"

// ============== 全局实例 ==============
Serial_Typedef Serial1;
#ifdef Serial2_Enable
Serial_Typedef Serial2;
#endif
#ifdef Serial4_Enable
Serial_Typedef Serial4;
#endif

// ============== 内部函数声明 ==============
static Serial_Typedef* Serial_GetInstance(UART_HandleTypeDef *huart);
static void Serial_Parse_HEX(Serial_Typedef *pSerial, uint16_t Size);
static void Serial_Parse_ABC(Serial_Typedef *pSerial);

// ============== 初始化 ==============
void Serial_Init(void)
{
    // ----- Serial1 -----
    Serial1.Instance = USART1;
    Serial1.huart = &huart1;
    Serial1.rxLen = 0;
    memset(Serial1.rxBuf, 0, Serial_RX_BUF_SIZE);
    memset(&Serial1.ABC_Data, 0, sizeof(Serial1.ABC_Data));
    memset(&Serial1.HEX_Data, 0, sizeof(Serial1.HEX_Data));
    HAL_UARTEx_ReceiveToIdle_DMA(Serial1.huart, Serial1.rxBuf, Serial_RX_BUF_SIZE);

#ifdef Serial2_Enable
    // ----- Serial2 -----
    Serial2.Instance = USART2;
    Serial2.huart = &huart2;
    Serial2.rxLen = 0;
    memset(Serial2.rxBuf, 0, Serial_RX_BUF_SIZE);
    memset(&Serial2.ABC_Data, 0, sizeof(Serial2.ABC_Data));
    memset(&Serial2.HEX_Data, 0, sizeof(Serial2.HEX_Data));
    HAL_UARTEx_ReceiveToIdle_DMA(Serial2.huart, Serial2.rxBuf, Serial_RX_BUF_SIZE);
#endif

#ifdef Serial4_Enable
    // ----- Serial4 -----
    Serial4.Instance = UART4;
    Serial4.huart = &huart4;
    Serial4.rxLen = 0;
    memset(Serial4.rxBuf, 0, Serial_RX_BUF_SIZE);
    memset(&Serial4.ABC_Data, 0, sizeof(Serial4.ABC_Data));
    memset(&Serial4.HEX_Data, 0, sizeof(Serial4.HEX_Data));
    HAL_UARTEx_ReceiveToIdle_DMA(Serial4.huart, Serial4.rxBuf, Serial_RX_BUF_SIZE);
#endif

    // 初始化协议常量
    Serial_Agreement_ABC_Init();
    Serial_Agreement_HEX_Init();
}

// ============== 发送（阻塞式printf）==============
void Serial_printf(Serial_Typedef *pSerial, const char *fmt, ...)
{
    char buffer[256];
    va_list args;
    int len;

    va_start(args, fmt);
    len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (len > 0) {
        if (len >= (int)sizeof(buffer))
            len = sizeof(buffer) - 1;
        HAL_UART_Transmit_DMA(pSerial->huart, (uint8_t *)buffer, len);
        while (__HAL_DMA_GET_COUNTER(pSerial->huart->hdmatx) != 0);
    }
}

// ========== HEX协议 ==========

// ============== HEX协议：解析 ==============
// 帧格式：[0xFF][0xAA][LEN][D1_H][D1_L][D1_CK][D2_H][D2_L][D2_CK]...[0x55][0xFE]
static void Serial_Parse_HEX(Serial_Typedef *pSerial, uint16_t Size)
{
    uint8_t LEN = pSerial->rxBuf[2];

    // LEN超限检查
    if (LEN > Serial_RX_MAX_WORDS) {
        pSerial->HEX_Data.err = Serial_Err_HEX_Len_OverFlow;
        return;
    }

    // 数据完整性检查：帧头2 + LEN1 + 数据(LEN×3) + 帧尾2
    uint16_t needed_len = 2 + 1 + LEN * 3 + 2;
    if (Size < needed_len) return;

    // 帧尾检查
    uint16_t tail_idx = 3 + LEN * 3;
    if (pSerial->rxBuf[tail_idx] != Serial_Agreement_HEX.end1 ||
        pSerial->rxBuf[tail_idx + 1] != Serial_Agreement_HEX.end2) {
        pSerial->HEX_Data.err = Serial_Err_HEX_Tail;
        return;
    }

    // 逐字解析（校验失败保留原值）
    for (uint8_t i = 0; i < LEN; i++) {
        uint8_t dh = pSerial->rxBuf[3 + i * 3];
        uint8_t dl = pSerial->rxBuf[3 + i * 3 + 1];
        uint8_t ck = pSerial->rxBuf[3 + i * 3 + 2];
        if ((dh ^ dl) == ck) {
            pSerial->HEX_Data.data[i] = (int16_t)((dh << 8) | dl);
        }
    }

    pSerial->HEX_Data.len = LEN;
    pSerial->HEX_Data.frame_valid = true;
}

// ============== HEX协议：获取数据 ==============
int16_t Serial_GetHexData(Serial_Typedef *pSerial, uint8_t index)
{
    if (index >= Serial_RX_MAX_WORDS)
        return 0;
    return pSerial->HEX_Data.data[index];
}

uint8_t Serial_GetHexLen(Serial_Typedef *pSerial)
{
    return pSerial->HEX_Data.len;
}

uint8_t Serial_GetNewPackageFlag_HEX(Serial_Typedef *pSerial)
{
    if (pSerial->HEX_Data.frame_valid == true) {
        pSerial->HEX_Data.frame_valid = false;
        return 1;
    }
    return 0;
}

int Serial_GetError_HEX(Serial_Typedef *pSerial)
{
    return (int)pSerial->HEX_Data.err;
}

// ========== ABC协议 ==========

// ============== ABC协议：解析 ==============
static void Serial_Parse_ABC(Serial_Typedef *pSerial)
{
    // 1. 检测帧头
    if (pSerial->rxBuf[0] != Serial_Agreement_ABC.head) {
        pSerial->ABC_Data.err = Serial_Err_ABC_Head;
        return;
    }

    // 2. 边处理数据边检测帧尾
    int i = 0;
    for (i = 1; pSerial->rxBuf[i + 1] != Serial_Agreement_ABC.end1; i++) {
        pSerial->ABC_Data.Serial_New_Package_ABC[i - 1] = pSerial->rxBuf[i];
        if (i > Serial_Wait_Tail_MAX)
            break;
    }
    pSerial->ABC_Data.Serial_New_Package_ABC[i - 1] = pSerial->rxBuf[i];

    // 3. 检测第1个帧尾
    if (pSerial->rxBuf[i + 1] == Serial_Agreement_ABC.end1) {
        // 4. 检测第2个帧尾
        if (pSerial->rxBuf[i + 2] != Serial_Agreement_ABC.end2) {
            pSerial->ABC_Data.err = Serial_Err_ABC_Tail;
            memset(pSerial->ABC_Data.Serial_New_Package_ABC, 0, sizeof(pSerial->ABC_Data.Serial_New_Package_ABC));
            return;
        } else {
            pSerial->ABC_Data.Serial_New_Package_ABC[i] = '\0';
            pSerial->ABC_Data.err = Serial_Err_None;
            pSerial->ABC_Data.Serial_New_Package_Flag = 1;
        }
    } else {
        pSerial->ABC_Data.err = Serial_Err_ABC_Tail;
        memset(pSerial->ABC_Data.Serial_New_Package_ABC, 0, sizeof(pSerial->ABC_Data.Serial_New_Package_ABC));
    }
}

// ============== ABC协议：获取新包标志 ==============
uint8_t Serial_GetNewPackageFlag_ABC(Serial_Typedef *pSerial)
{
    if (pSerial->ABC_Data.Serial_New_Package_Flag == 1) {
        pSerial->ABC_Data.Serial_New_Package_Flag = 0;
        return 1;
    }
    return 0;
}

int Serial_GetError_ABC(Serial_Typedef *pSerial)
{
    return (int)pSerial->ABC_Data.err;
}

// ============== ABC协议：设置浮点数 ==============
bool Serial_SetFloatData(Serial_Typedef *pSerial, char *KeyWord, char *cmd, float *Data)
{
    if (strstr(pSerial->ABC_Data.Serial_New_Package_ABC, KeyWord) != NULL) {
        sscanf(pSerial->ABC_Data.Serial_New_Package_ABC, cmd, Data);
        return true;
    }
    return false;
}

// ============== ABC协议：设置整数 ==============
bool Serial_SetIntData(Serial_Typedef *pSerial, char *KeyWord, char *cmd, int *Data)
{
    if (strstr(pSerial->ABC_Data.Serial_New_Package_ABC, KeyWord) != NULL) {
        sscanf(pSerial->ABC_Data.Serial_New_Package_ABC, cmd, Data);
        return true;
    }
    return false;
}

// ============== ABC协议：检测指令关键字 ==============
bool Serial_Check_Str(Serial_Typedef *pSerial, char *KeyWord)
{
    if (strstr(pSerial->ABC_Data.Serial_New_Package_ABC, KeyWord) != NULL)
        return true;
    return false;
}

// ========== 空闲中断回调 ==========

// ============== 根据huart查找Serial实例 ==============
static Serial_Typedef* Serial_GetInstance(UART_HandleTypeDef *huart)
{
    if (huart->Instance == Serial1.Instance) return &Serial1;
#ifdef Serial2_Enable
    if (huart->Instance == Serial2.Instance) return &Serial2;
#endif
#ifdef Serial4_Enable
    if (huart->Instance == Serial4.Instance) return &Serial4;
#endif
    return NULL;
}

// ============== 串口空闲中断回调（统一入口）==============
// HEX协议帧格式：
//   [0xFF][0xAA][LEN][D1_H][D1_L][D1_CK][D2_H][D2_L][D2_CK]...[0x55][0xFE]
//
// 变量说明：
//   Size       — 本次Idle中断接收到的总字节数
//   LEN       — 帧中第3字节，表示"字个数"（每个字=1个int16_t=2字节原始数据）
//   needed_len — 帧完整所需的最小字节数 = 帧头2 + LEN1 + 数据(LEN×3) + 帧尾2
//   tail_idx  — 帧尾起始位置 = 3 + LEN*3（字节偏移量）

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    // 1. 查找实例
    Serial_Typedef *pSerial = Serial_GetInstance(huart);
    if (pSerial == NULL) return;

    // 2. 清除状态
    pSerial->rxLen = Size;
    pSerial->HEX_Data.frame_valid = false;
    pSerial->HEX_Data.err = Serial_Err_None;
    pSerial->ABC_Data.err = Serial_Err_None;

    // 最小长度检查：帧头2 + LEN1 + 帧尾2 = 5字节
    if (Size < 5) goto _restart;

    // 3. 协议分发
    if (pSerial->rxBuf[0] == Serial_Agreement_HEX.head1 &&
        pSerial->rxBuf[1] == Serial_Agreement_HEX.head2) {
        Serial_Parse_HEX(pSerial, Size);
    }
    else if (pSerial->rxBuf[0] == Serial_Agreement_ABC.head) {
        Serial_Parse_ABC(pSerial);
    }
    else {
        pSerial->ABC_Data.err = Serial_Err_ABC_Head;
    }

_restart:
    HAL_UARTEx_ReceiveToIdle_DMA(pSerial->huart, pSerial->rxBuf, Serial_RX_BUF_SIZE);
}
