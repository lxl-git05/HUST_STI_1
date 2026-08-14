#include "Serial_porting.h"
#include "string.h"
#include <stdarg.h>
#include <stdio.h>

// ============== 全局实例 ==============
Serial_Typedef Serial1;
#ifdef Serial2_Enable
Serial_Typedef Serial2;
#endif
#ifdef Serial3_Enable
Serial_Typedef Serial3;
#endif
#ifdef Serial4_Enable
Serial_Typedef Serial4;
#endif

// ============== 内部函数声明 ==============
static Serial_Typedef* Serial_GetInstance(UART_HandleTypeDef *huart);
static void Serial_Parse_HEX(Serial_Typedef *pSerial, uint16_t Size);
static void Serial_Parse_ABC(Serial_Typedef *pSerial);

// ============== 接收/发送统一入口 ==============
// 启动接收：有 DMA 接收句柄的串口走 DMA，否则中断接收（自动适配 CubeMX 的 DMA 配置）
static void Serial_StartRx(Serial_Typedef *pSerial)
{
    if (pSerial->huart->hdmarx != NULL)
        HAL_UARTEx_ReceiveToIdle_DMA(pSerial->huart, pSerial->rxBuf, Serial_RX_BUF_SIZE);
    else
        HAL_UARTEx_ReceiveToIdle_IT(pSerial->huart, pSerial->rxBuf, Serial_RX_BUF_SIZE);
}

// 统一发送入口：有 DMA 发送句柄的串口走 DMA，否则阻塞发送
static void Serial_TX(Serial_Typedef *pSerial, uint8_t *buf, uint16_t len, uint32_t timeout)
{
    if (pSerial->huart->hdmatx != NULL && len > 0 && len <= Serial_TX_BUF_SIZE)
    {
        // 等待上一次 DMA 发送完成。
        // 注意：DMA 完成中断与定时器中断同为抢占优先级0，中断上下文里 DMA 中断
        //       无法抢占本中断，若忙则直接丢弃本帧，防止死等
        if (__get_IPSR() != 0)
        {
            if (pSerial->huart->gState == HAL_UART_STATE_BUSY_TX) return;
        }
        else
        {
            while (pSerial->huart->gState == HAL_UART_STATE_BUSY_TX) { }
        }
        memcpy(pSerial->txBuf, buf, len);
        // H7 D-Cache：DMA 读内存不经过缓存，发送前必须把缓冲回写内存
        SCB_CleanDCache_by_Addr((uint32_t *)pSerial->txBuf, Serial_TX_BUF_SIZE);
        HAL_UART_Transmit_DMA(pSerial->huart, pSerial->txBuf, len);
    }
    else
    {
        HAL_UART_Transmit(pSerial->huart, buf, len, timeout);   // 阻塞发送
    }
}

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
    Serial_StartRx(&Serial1);

#ifdef Serial2_Enable
    // ----- Serial2 -----
    Serial2.Instance = USART2;
    Serial2.huart = &huart2;
    Serial2.rxLen = 0;
    memset(Serial2.rxBuf, 0, Serial_RX_BUF_SIZE);
    memset(&Serial2.ABC_Data, 0, sizeof(Serial2.ABC_Data));
    memset(&Serial2.HEX_Data, 0, sizeof(Serial2.HEX_Data));
    Serial_StartRx(&Serial2);
#endif

#ifdef Serial3_Enable
    // ----- Serial3 -----
    Serial3.Instance = USART3;
    Serial3.huart = &huart3;
    Serial3.rxLen = 0;
    memset(Serial3.rxBuf, 0, Serial_RX_BUF_SIZE);
    memset(&Serial3.ABC_Data, 0, sizeof(Serial3.ABC_Data));
    memset(&Serial3.HEX_Data, 0, sizeof(Serial3.HEX_Data));
    Serial_StartRx(&Serial3);
#endif

#ifdef Serial4_Enable
    // ----- Serial4 -----
    Serial4.Instance = UART4;
    Serial4.huart = &huart4;
    Serial4.rxLen = 0;
    memset(Serial4.rxBuf, 0, Serial_RX_BUF_SIZE);
    memset(&Serial4.ABC_Data, 0, sizeof(Serial4.ABC_Data));
    memset(&Serial4.HEX_Data, 0, sizeof(Serial4.HEX_Data));
    Serial_StartRx(&Serial4);
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
        Serial_TX(pSerial, (uint8_t *)buffer, len, 100);
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

// 精确匹配（strcmp）
bool Serial_CheckCmd(Serial_Typedef *pSerial, char *cmd)
{
    return (strcmp(pSerial->ABC_Data.Serial_New_Package_ABC, cmd) == 0);
}

// ========== 发送 API ==========

// 轮询发字符串（阻塞式）
void Serial_send_string(Serial_Typedef *pSerial, char *str)
{
    uint16_t len = (uint16_t)strlen(str);
    if (len > 0)
        Serial_TX(pSerial, (uint8_t *)str, len, 1000);
}

// 发送原始字节数组（阻塞式）
void Serial_SendBytes(Serial_Typedef *pSerial, uint8_t *buf, uint16_t len)
{
    if (len > 0)
        Serial_TX(pSerial, buf, len, 1000);
}

// 发送 HEX 协议包（帧头+LEN+数据[XOR校验]+帧尾）
void Serial_Send_HEX_Package(Serial_Typedef *pSerial, uint16_t *data, uint8_t count)
{
    uint8_t txBuf[3 + 256 * 3 + 2]; // 最大帧大小
    uint16_t idx = 0;

    txBuf[idx++] = Serial_Agreement_HEX.head1;
    txBuf[idx++] = Serial_Agreement_HEX.head2;
    txBuf[idx++] = count;                           // LEN = 字数

    for (uint8_t i = 0; i < count; i++)
    {
        uint8_t dh = (data[i] >> 8) & 0xFF;
        uint8_t dl = data[i] & 0xFF;
        txBuf[idx++] = dh;
        txBuf[idx++] = dl;
        txBuf[idx++] = dh ^ dl;                     // XOR 校验码
    }

    txBuf[idx++] = Serial_Agreement_HEX.end1;
    txBuf[idx++] = Serial_Agreement_HEX.end2;

    Serial_SendBytes(pSerial, txBuf, idx);
}

// ========== 工具 API ==========

// 清空 ABC 接收缓冲区
void Serial_Clear_ABC(Serial_Typedef *pSerial)
{
    memset(pSerial->ABC_Data.Serial_New_Package_ABC, 0,
           sizeof(pSerial->ABC_Data.Serial_New_Package_ABC));
    pSerial->ABC_Data.Serial_New_Package_Flag = 0;
    pSerial->ABC_Data.err = Serial_Err_None;
}

// 打印调试统计
void Serial_PrintDebug(Serial_Typedef *pSerial)
{
    Serial_printf(pSerial,
        "=== Serial Debug ===\r\n"
        "  Instance: 0x%08lX\r\n"
        "==================\r\n",
        (uint32_t)pSerial->Instance);
}

// ========== 空闲中断回调 ==========

// ============== 根据huart查找Serial实例 ==============
static Serial_Typedef* Serial_GetInstance(UART_HandleTypeDef *huart)
{
    if (huart->Instance == Serial1.Instance) return &Serial1;
#ifdef Serial2_Enable
    if (huart->Instance == Serial2.Instance) return &Serial2;
#endif
#ifdef Serial3_Enable
    if (huart->Instance == Serial3.Instance) return &Serial3;
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

    // DMA 接收缓存维护：DMA 直接写内存不经过缓存，CPU 读 rxBuf 前必须先失效 D-Cache
    if (pSerial->huart->hdmarx != NULL)
        SCB_InvalidateDCache_by_Addr((uint32_t *)pSerial->rxBuf, Serial_RX_BUF_SIZE);

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
    Serial_StartRx(pSerial);
}
