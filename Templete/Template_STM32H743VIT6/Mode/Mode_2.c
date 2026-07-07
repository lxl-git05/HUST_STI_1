#include "Mode_2.h"
#include "AllHeader.h"

void Mode_2_Setup(void)
{
    OLED_Clear();
}

// ============== 串口自发自收测试全局变量 ==============
static int16_t send_data[4] = {0, 0, 0, 0};  // 发送数据
static int16_t recv_data[4] = {0, 0, 0, 0};  // 接收数据（Serial1和Serial2共用）

// ============== HEX帧发送函数 ==============
static void Serial_HEX_Send(Serial_Typedef *pSerial, int16_t *data, uint8_t len)
{
    uint8_t txBuf[Serial_RX_BUF_SIZE];
    uint8_t idx = 0;

    txBuf[idx++] = 0xFF;
    txBuf[idx++] = 0xAA;
    txBuf[idx++] = len;

    for (uint8_t i = 0; i < len; i++) {
        txBuf[idx++] = (uint8_t)(data[i] >> 8);
        txBuf[idx++] = (uint8_t)(data[i] & 0xFF);
        txBuf[idx++] = (uint8_t)(data[i] >> 8) ^ (uint8_t)(data[i] & 0xFF);
    }

    txBuf[idx++] = 0x55;
    txBuf[idx++] = 0xFE;

    HAL_UART_Transmit(pSerial->huart, txBuf, idx, 100);
}

// ============== 接收处理（Serial1和Serial2共用）==============
static void Serial_Recv_Handle(Serial_Typedef *pSerial)
{
    if (Serial_GetNewPackageFlag_HEX(pSerial)) {
        for (uint8_t i = 0; i < 4; i++) {
            recv_data[i] = Serial_GetHexData(pSerial, i);
        }
    }
}

// ============== Mode_2 主循环 ==============
void Mode_2_Loop(void)
{
    // 按键1：Serial1发送
    if (Key_Check(KEY_0, KEY_SINGLE)) {
        Serial_HEX_Send(&Serial1, send_data, 4);
    }

    // 按键2：Serial2发送
#ifdef Serial2_Enable
    if (Key_Check(KEY_1, KEY_SINGLE)) {
        Serial_HEX_Send(&Serial2, send_data, 4);
    }
#endif

    // 按键3：数据增加
    if (Key_Check(KEY_2, KEY_SINGLE)) {
        send_data[0] += 1;
        send_data[1] += 100;
        send_data[2] += 1000;
        send_data[3] += 0x55;
    }

    // 按键0：数据减少
    if (Key_Check(KEY_2, KEY_DOUBLE)) {
        send_data[0] -= 1;
        send_data[1] -= 100;
        send_data[2] -= 0xFF;
        send_data[3] -= 0x55;
    }

    // Serial1和Serial2接收（共用recv_data）
    Serial_Recv_Handle(&Serial1);
#ifdef Serial2_Enable
    Serial_Recv_Handle(&Serial2);
#endif

    // OLED显示
    OLED_Printf(0, 0, OLED_6X8, "S:%d,%d,%d,%d", send_data[0], send_data[1], send_data[2], send_data[3]);
    OLED_Printf(0, 20, OLED_6X8, "R:%d,%d,%d,%d", recv_data[0], recv_data[1], recv_data[2], recv_data[3]);
}

void Mode_2_Tick(void)
{
}

void Mode_2_Exit(void)
{
}
