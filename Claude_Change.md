
## 2026-08-14 13:37 | 加入 6 路舵机驱动（Servo + Con_Servo）

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Servo.h | ./Hardware/Servo.h | 新增 | 简化舵机驱动头文件（180°、双限幅字段、Init/SetAngle/Get_Angle 三 API） |
| Servo.c | ./Hardware/Servo.c | 新增 | 驱动实现：50Hz 校验 + 双限幅 + 线性映射 + 初始角度写入 |
| Con_Servo.h | ./Function/Con_Servo.h | 新增 | 6 路舵机实例 extern + Con_Servo_Init 声明 |
| Con_Servo.c | ./Function/Con_Servo.c | 新增 | 6 路实例定义，统一初始 90°（限幅 0~180°） |
| MyPWM.h | ./MySystem/MyPWM.h | 修改 | 补 MyPWM_Servo5/6 extern 声明 |
| MySystem.c | ./MySystem/MySystem.c | 修改 | 定义 6 路舵机 PWM 实例（TIM1 CH1-4 + TIM8 CH3/4），限幅修正为 250/50 tick |
| AllHeader.h | ./Top/AllHeader.h | 修改 | 硬件驱动库组加 Servo.h，硬件实现库组加 Con_Servo.h |
| AllHeader.c | ./Top/AllHeader.c | 修改 | Initial_ALL 调用 Con_Servo_Init（开机 6 路归中 90°） |

> Keil 工程添加 Servo.c / Con_Servo.c 由用户自行操作。

## 2026-08-14 14:00 | 基于 Mode_4 新建 Mode_5 / Mode_6 空骨架

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Mode_5.h | ./Mode/Mode_5.h | 新增 | Mode_4 骨架克隆（Setup/Loop/Tick/Exit 声明） |
| Mode_5.c | ./Mode/Mode_5.c | 新增 | Mode_4 骨架克隆（OLED 显示 ===Mode_5===） |
| Mode_6.h | ./Mode/Mode_6.h | 新增 | Mode_4 骨架克隆（Setup/Loop/Tick/Exit 声明） |
| Mode_6.c | ./Mode/Mode_6.c | 新增 | Mode_4 骨架克隆（OLED 显示 ===Mode_6===） |
| Mode_G.h | ./Mode/Mode_G.h | 修改 | 枚举新增 Mode_5 / Mode_6 |
| Mode_G.c | ./Mode/Mode_G.c | 修改 | 20ms Tick switch 新增 Mode_5 / Mode_6 分支 |
| Mymain.c | ./Top/Mymain.c | 修改 | 三处 switch（Loop/Exit/Setup）新增 Mode_5 / Mode_6 分支 |
| AllHeader.h | ./Top/AllHeader.h | 修改 | Mode 库组新增 Mode_5.h / Mode_6.h |

> Keil 工程添加 Mode_5.c / Mode_6.c 由用户自行操作。

## 2026-08-14 14:19 | USART1 改为 DMA 收发（RX=DMA1_Stream0, TX=DMA1_Stream1）

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Serial_porting.h | ./Function/Serial_porting.h | 修改 | rxBuf 改为 32 字节对齐并补齐整 cache line（800B），供 D-Cache 维护使用 |
| Serial_porting.c | ./Function/Serial_porting.c | 修改 | 新增 Serial_TX 统一发送入口：USART1 走 DMA（含 D-Cache 回写），其余串口保持阻塞；Serial1 接收改 ReceiveToIdle_DMA，回调失效缓存并分路重启；printf/send_string/SendBytes 全部改走 Serial_TX |

> 关键设计：0x24000000 区域可缓存 → TX 前 CleanDCache、RX 解析前 InvalidateDCache；中断上下文里 TX 忙时直接丢帧（DMA 完成中断与定时器同为抢占优先级0，无法抢占，等则死锁）。Keil 工程无需新增文件（dma.c/h 由 CubeMX 重新生成时已加入）。

## 2026-08-14 14:42 | 串口层 DMA 通用化（为全串口 DMA 做准备）

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Serial_porting.h | ./Function/Serial_porting.h | 修改 | 结构体新增 32 字节对齐 txBuf[256]；Serial_TX_BUF_SIZE 宏移到头文件 |
| Serial_porting.c | ./Function/Serial_porting.c | 修改 | 删 USART1 硬编码：新增 Serial_StartRx()（按 hdmarx 自动选 DMA/中断），Serial_TX 按 hdmatx 自动选 DMA/阻塞；4 路 Init、接收回调、重启全部通用化 |

> 用户后续在 CubeMX 给 USART2/3/UART4 加 DMA 重新生成后，代码自动切换 DMA 收发，无需再改。

## 2026-08-14 15:05 | 编写工程小结文档（供后续 Agent 快速上手）

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| 工程小结.md | ./Templete/Template_STM32H743VIT6/工程小结.md | 新增 | 全工程架构/API/硬件总表/关键坑总结（基于实际代码逐项核查） |
| CLAUDE.md | ./Templete/Template_STM32H743VIT6/CLAUDE.md | 修改 | 顶部加指针行，引导新会话先读工程小结.md |
