[toc]

# Serial 模块使用说明

## 1. 项目简介

* **功能**：实现单片机与外界（上位机/传感器/其他设备）的信息通信，支持 HEX 和 ABC 两种协议
* **特性**：
  * 支持多个串口（Serial1、Serial2等）
  * HEX 协议支持逐字校验，单字错误保留原值
  * ABC 协议支持上位机调参（VOFA）
  * Idle 中断接收，无需轮询

## 2. 文件结构

```
Hardware/
└── Serial_base.h/c    ← 协议定义、宏、数据结构

Function/
└── Serial_porting.h/c ← 外设实例、初始化、Idle中断、收发
```

## 3. 协议格式

### HEX协议（设备间通信）

```
[0xFF][0xAA][LEN][D1_H][D1_L][D1_CK][D2_H][D2_L][D2_CK]...[DN_H][DN_L][DN_CK][0x55][0xFE]
```

| 字段 | 字节数 | 说明 |
|------|--------|------|
| 帧头1 | 1 | `0xFF` |
| 帧头2 | 1 | `0xAA` |
| LEN | 1 | **字个数**（每个字 = 1个int16_t = 2字节原始数据） |
| D_H | 1 | 数据高8位 |
| D_L | 1 | 数据低8位 |
| D_CK | 1 | 校验码 = `D_H ^ D_L` |
| 帧尾1 | 1 | `0x55` |
| 帧尾2 | 1 | `0xFE` |

**校验规则**：
- 每字校验：`D_H ^ D_L == D_CK` → 写入数据，否则**保留原值不更新**

### ABC协议（上位机调参）

```
[@][DATA...][$][#]
```

| 字段 | 说明 |
|------|------|
| `@` | 帧头 |
| DATA | 指令内容，建议格式：`Kp=0.5` |
| `$#` | 帧尾 |

## 4. 核心API

### 4.1 初始化与发送

```c
// 初始化所有启用的串口（在 Initial_ALL 中调用）
void Serial_Init(void);

// 发送字符串（阻塞式）
void Serial_printf(Serial_Typedef *pSerial, const char *fmt, ...);

// 示例
Serial_printf(&Serial1, "Hello %d\r\n", 123);
```

### 4.2 HEX协议

```c
// 获取第index个字（返回int16_t）
int16_t Serial_GetHexData(Serial_Typedef *pSerial, uint8_t index);

// 获取实际接收的字个数
uint8_t Serial_GetHexLen(Serial_Typedef *pSerial);

// 获取新包标志（自动清除标志）
uint8_t Serial_GetNewPackageFlag_HEX(Serial_Typedef *pSerial);

// 获取HEX错误码
int Serial_GetError_HEX(Serial_Typedef *pSerial);

// 示例
if (Serial_GetNewPackageFlag_HEX(&Serial1)) {
    int16_t val0 = Serial_GetHexData(&Serial1, 0);
    int16_t val1 = Serial_GetHexData(&Serial1, 1);
}
```

### 4.3 ABC协议

```c
// 获取新包标志（自动清除标志）
uint8_t Serial_GetNewPackageFlag_ABC(Serial_Typedef *pSerial);

// 获取ABC错误码
int Serial_GetError_ABC(Serial_Typedef *pSerial);

// 解析浮点数（KeyWord为关键词，cmd为格式字符串）
bool Serial_SetFloatData(Serial_Typedef *pSerial, char *KeyWord, char *cmd, float *Data);

// 解析整数
bool Serial_SetIntData(Serial_Typedef *pSerial, char *KeyWord, char *cmd, int *Data);

// 检测指令关键字
bool Serial_Check_Str(Serial_Typedef *pSerial, char *KeyWord);

// 示例：VOFA发送 @Kp=0.5$#
if (Serial_GetNewPackageFlag_ABC(&Serial1)) {
    Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &Kp);
    Serial_SetFloatData(&Serial1, "Ki", "Ki=%f", &Ki);
    Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &Kd);
}
```

### 4.4 错误码

```c
// HEX错误码
Serial_Err_None              = 0x00  // 无错误
Serial_Err_HEX_Head           = 0x10 // HEX帧头错误
Serial_Err_HEX_Tail          = 0x20  // HEX帧尾错误
Serial_Err_HEX_Len_OverFlow  = 0x30  // LEN超过上限

// ABC错误码
Serial_Err_ABC_Head           = 0x01  // ABC帧头错误
Serial_Err_ABC_Tail           = 0x02  // ABC帧尾错误
```

## 5. 多串口配置

### 启用Serial2

1. 在 `Serial_porting.h` 中取消注释：
```c
#define Serial2_Enable 1
```

2. 在 CubeMX 中配置 USART2（引脚 PA2=USART2_TX, PA3=USART2_RX）

3. 初始化自动完成，无需额外代码

### 多串口使用示例

```c
// Serial1发送
Serial_printf(&Serial1, "Serial1\r\n");

// Serial2发送
Serial_printf(&Serial2, "Serial2\r\n");

// Serial1接收
if (Serial_GetNewPackageFlag_HEX(&Serial1)) {
    // 处理Serial1数据
}

// Serial2接收
if (Serial_GetNewPackageFlag_HEX(&Serial2)) {
    // 处理Serial2数据
}
```

## 6. 引脚定义

| 引脚号 | 标签 | 说明 |
|--------|------|------|
| PA9 | USART1_TX | 串口1发送 |
| PA10 | USART1_RX | 串口1接收 |
| PA2 | USART2_TX | 串口2发送 |
| PA3 | USART2_RX | 串口2接收 |

## 7. 使用示例

### HEX自发自收测试

```c
// Mode_2中的测试代码
static int16_t send_data[4] = {1, 2, 3, 4};
static int16_t recv_data[4] = {0, 0, 0, 0};

// HEX帧发送函数
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

// 发送
Serial_HEX_Send(&Serial1, send_data, 4);

// 接收
if (Serial_GetNewPackageFlag_HEX(&Serial1)) {
    for (uint8_t i = 0; i < 4; i++) {
        recv_data[i] = Serial_GetHexData(&Serial1, i);
    }
}
```

### ABC调参示例

```c
float Kp = 0.0f, Ki = 0.0f, Kd = 0.0f;

// 在循环中
if (Serial_GetNewPackageFlag_ABC(&Serial1)) {
    Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &Kp);
    Serial_SetFloatData(&Serial1, "Ki", "Ki=%f", &Ki);
    Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &Kd);
}

// VOFA发送指令格式：@Kp=0.5$#
```

## 8. 注意事项

### 8.1 数据类型

- `Serial_GetHexData` 返回 `int16_t`
- 如果数据是无符号的（0~32767），直接当正数使用即可
- 如果需要显示为无符号：`%u` 打印或强转 `(uint16_t)`

### 8.2 多串口隔离

- Serial1 和 Serial2 的接收数据**不隔离**，共用同一套业务逻辑
- 如需隔离处理，使用 `ifdef Serial2_Enable` 条件编译

### 8.3 帧尾位置计算

```
帧尾位置 = 3 + LEN * 3  // 字节偏移量
完整帧字节数 = 2 + 1 + LEN * 3 + 2
```
