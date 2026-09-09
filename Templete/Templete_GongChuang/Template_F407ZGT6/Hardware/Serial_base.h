#ifndef __SERIAL_BASE_H
#define __SERIAL_BASE_H

#include "MySystem.h"

// ============== 宏定义 ==============
// ========== HEX协议相关 ==========
// 注意：以下"字"指 int16_t 数据（高低位合成后的完整数据）
//       "高低位对"指 D_H + D_L（1个字占3字节：高位+低位+校验码）

#define Serial_RX_MAX_WORDS  256   	 // 最大接收"字"的个数（高低位对的个数）
                                     // 例：LEN=10 表示接收10个int16_t数据（占30字节数据区）

#define Serial_RX_BUF_SIZE   (3 * Serial_RX_MAX_WORDS + 5)
                                     // 接收缓冲区总字节数 = 帧头2 + LEN1 + 数据(N*3) + 帧尾2
                                     // 其中 N = Serial_RX_MAX_WORDS

// ========== ABC协议相关 ==========
#define Serial_ABC_BUF_SIZE      40   // ABC字符串接收缓冲区大小（字节）
#define Serial_Wait_Tail_MAX     25   // ABC帧尾等待超时阈值（防止死循环）

// ============== 错误码定义 ==============
typedef enum {
    Serial_Err_None = 0x00,

    // ABC错误（0x01~0x0F）
    Serial_Err_ABC_Head = 0x01,      // ABC帧头错误
    Serial_Err_ABC_Tail = 0x02,      // ABC帧尾错误

    // HEX错误（0x10~0xFF）
    Serial_Err_HEX_Head = 0x10,      // HEX帧头错误
    Serial_Err_HEX_Tail = 0x20,      // HEX帧尾错误
    Serial_Err_HEX_Len_OverFlow = 0x30, // LEN值超过Serial_RX_MAX_WORDS上限
} Serial_Error_Typedef;

// ============== ABC协议数据 ==============
typedef struct {
    char Serial_New_Package_ABC[Serial_ABC_BUF_SIZE]; // 字符串接收缓冲区（字节数组）
    bool Serial_New_Package_Flag;                      // 新数据包标志（收到完整帧置1）
    Serial_Error_Typedef err;                          // 错误状态
} Serial_ABC_Data_Typedef;

// ============== HEX协议数据 ==============
// 帧格式：[0xFF][0xAA][LEN][D1_H][D1_L][D1_CK][D2_H][D2_L][D2_CK]...[DN_H][DN_L][DN_CK][0x55][0xFE]
//         ─────── ─────── ──── ────────────────────────────────────────────────────────  ───────
//         帧头1 帧头2  1B  ├───────────────── 数据区 N×3字节 ─────────────────┤         帧尾1 帧尾2
//
// 说明：
//   LEN      — 字个数（1字节），表示后续有多少个"高低位对"
//   D_X_H    — 第X个字的高8位
//   D_X_L    — 第X个字的低8位
//   D_X_CK   — 第X个字的校验码（ D_X_H ^ D_X_L）
//   数据区   — 共 LEN×3 字节
//   帧尾     — 0x55 0xFE

typedef struct {
    int16_t data[Serial_RX_MAX_WORDS];    // 解析结果数组（每个元素是1个完整的int16_t）
                                          // 注意：校验失败的字位置保留原值，不更新
    uint8_t len;                          // 实际接收的字个数（与帧中LEN字段一致）
    bool frame_valid;                     // 本帧是否有效（帧头帧尾检测通过=true）
    Serial_Error_Typedef err;             // 错误状态
} Serial_HEX_Data_Typedef;

// ============== 协议帧定义（常量）==============
// ABC协议帧格式：[@][DATA...][$][#]
typedef struct {
    uint8_t head;   // '@' 帧头
    uint8_t end1;   // '$' 帧尾1
    uint8_t end2;   // '#' 帧尾2
} Serial_Agreement_ABC_TypeDef;

// HEX协议帧格式：[0xFF][0xAA][LEN][DATA...][0x55][0xFE]
typedef struct {
    uint8_t head1;  // 0xFF 帧头1
    uint8_t head2;  // 0xAA 帧头2
    uint8_t end1;   // 0x55 帧尾1
    uint8_t end2;   // 0xFE 帧尾2
} Serial_Agreement_HEX_TypeDef;

// 外部声明（协议常量实例）
extern Serial_Agreement_ABC_TypeDef Serial_Agreement_ABC;
extern Serial_Agreement_HEX_TypeDef Serial_Agreement_HEX;

// ============== 函数声明 ==============
// ABC协议初始化
void Serial_Agreement_ABC_Init(void);
// HEX协议初始化
void Serial_Agreement_HEX_Init(void);

#endif // !__SERIAL_BASE_H
