## 2026-07-07 15:00 | 创建项目CLAUDE.md文档

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| CLAUDE.md | ./CLAUDE.md | 新增 | 项目长期记忆文档，记录架构、设计规范、移植步骤 |

## 2026-07-07 16:50 | Serial模块重构

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Serial_base.h | ./Hardware/Serial_base.h | 重写 | 协议定义、宏、数据结构；HEX逐字校验协议 |
| Serial_base.c | ./Hardware/Serial_base.c | 重写 | 协议初始化函数 |
| Serial_porting.h | ./Function/Serial_porting.h | 新增 | 外设实例结构体、API声明 |
| Serial_porting.c | ./Function/Serial_porting.c | 新增 | 实现（Idle中断、ABC/HEX解析、发送） |
| Serial.c | ./Function/Serial.c | 删除 | 旧文件，已被Serial_porting.c替代 |
| Serial.h | ./Function/Serial.h | 删除 | 旧文件，已被Serial_porting.h替代 |
| AllHeader.h | ./Top/AllHeader.h | 修改 | Serial.h → Serial_porting.h |
| AllHeader.c | ./Top/AllHeader.c | 修改 | Serial_Init() → Serial_Porting_Init() |
| Mode_2.c | ./Mode/Mode_2.c | 修改 | 使用新HEX API（Serial_GetHexData等） |
| Template.uvprojx | ./MDK-ARM/Template.uvprojx | 修改 | Serial.c→Serial_porting.c |
| CLAUDE.md | ./CLAUDE.md | 修改 | 新增Serial模块文档 |

## 2026-07-07 17:30 | Serial_porting代码重构（可读性+多串口）

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Serial_porting.c | ./Function/Serial_porting.c | 重写 | HEX/ABC函数分组；封装Serial_Parse_HEX/ABC；Serial_GetInstance多串口支持 |
| Serial_porting.h | ./Function/Serial_porting.h | 修改 | 添加Serial2_Enable支持 |


## 2026-08-14 20:13 | F407→H743 移植全部 17 步完成（调参体系/任务队列/IMU/串口屏/Mode_1）

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| MyGPIO.h | ./MySystem/MyGPIO.h | 修改 | 新增 extern MyGPIO_EC11_Key(PB0) |
| MySystem.c | ./MySystem/MySystem.c | 修改 | 新增 MyGPIO_EC11_Key 实例定义 |
| MyTimer.h | ./MySystem/MyTimer.h | 修改 | 新增 Timer_DisableIRQ/EnableIRQ 声明 |
| MyTimer.c | ./MySystem/MyTimer.c | 修改 | 新增 Timer_DisableIRQ/EnableIRQ 实现 |
| MySystem.h | ./MySystem/MySystem.h | 修改 | 取消注释 i2c.h（IMU 用 hi2c1） |
| Key.c | ./Hardware/Key.c | 修改 | 恢复 KEY_3 分支 → MyGPIO_EC11_Key |
| Encoder_Key.h | ./Hardware/Encoder_Key.h | 新增 | EC11 旋转编码器驱动头文件 |
| Encoder_Key.c | ./Hardware/Encoder_Key.c | 新增 | 运行时重配 PC4/PC5 为 IT_FALLING + NVIC 使能 + 判向 |
| stm32h7xx_it.c | ./Core/Src/stm32h7xx_it.c | 修改 | USER CODE 区新增 EXTI4/EXTI9_5 IRQHandler |
| Serial_porting.h | ./Function/Serial_porting.h | 修改 | 新增 Serial3/Serial4 实例与 6 个 F4 API 声明 |
| Serial_porting.c | ./Function/Serial_porting.c | 修改 | Serial3(USART3)/Serial4(UART4) 中断接收 + 阻塞发送 API 实现 |
| TJC_LCD.h | ./Hardware/TJC_LCD.h | 新增 | 串口屏驱动头文件（F4 拷贝） |
| TJC_LCD.c | ./Hardware/TJC_LCD.c | 新增 | 串口屏指令解析与波形发送（默认 Serial4） |
| bsp_at24c02.h | ./AT/bsp_at24c02.h | 新增 | 软件 I2C AT24C02 底层（延时改 Delay_Us） |
| bsp_at24c02.c | ./AT/bsp_at24c02.c | 新增 | 软件 I2C 时序实现 |
| at24c02_manager.h | ./AT/at24c02_manager.h | 新增 | 参数注册管理器头文件（AT_PARAM_* 宏） |
| at24c02_manager.c | ./AT/at24c02_manager.c | 新增 | 地址自动分配 + 读写/批量读写 |
| Param_AT24C02.h | ./AT/Param_AT24C02.h | 新增 | 业务层头文件（PARAM_FORCE 宏） |
| Param_AT24C02.c | ./AT/Param_AT24C02.c | 新增 | 注册 s_AT_Params + 上电恢复 |
| ParamEdit.h | ./AT/ParamEdit.h | 新增 | 调参 UI 头文件（按键映射 KEY_1 进出/KEY_3 保存） |
| ParamEdit.c | ./AT/ParamEdit.c | 新增 | EC11 调参状态机 + OLED 渲染（删 Orange 依赖） |
| IMU/ 14 文件 | ./IMU/ | 新增 | F4 拷贝 13 文件 + MPU6050_base.c 的 I2C 状态机重写为 H7 v2 |
| Queue.h | ./Software/Queue.h | 新增 | 通用环形队列 |
| Queue.c | ./Software/Queue.c | 新增 | 通用环形队列实现 |
| Con_Task.h | ./Function/Con_Task.h | 新增 | 任务队列调度器（枚举裁剪 6 项，顺序不可改） |
| Con_Task.c | ./Function/Con_Task.c | 新增 | Setup→Run→IsExit→自动出队调度 + 20ms Tick 分发 |
| Control.h | ./Function/Control.h | 新增 | 5 个通用任务声明 + 全局任务表 |
| Control.c | ./Function/Control.c | 新增 | 等待/电机速度/电机角度任务实现（H743 签名适配） |
| Buzzer.h | ./Hardware/Buzzer.h | 修改 | 补 Buzzer_On/Buzzer_OFF 声明 |
| MyPID.h | ./Software/MyPID.h | 修改 | 新增 PID_Param_Reset 声明 |
| MyPID.c | ./Software/MyPID.c | 修改 | 新增 PID_Param_Reset 实现（清 PID 历史） |
| Menu_Param.h | ./Function/Menu_Param.h | 新增 | 调参任务菜单（仅 GYRO_CAL=0/GYRO_YAW=1） |
| Menu_Param.c | ./Function/Menu_Param.c | 新增 | 陀螺标定(写零偏入 EEPROM) + yaw 角环(内置 PID 差速) |
| Mode_G.h | ./Mode/Mode_G.h | 修改 | Mode_PID 更名 Mode_1 |
| Mode_G.c | ./Mode/Mode_G.c | 修改 | 模式记忆恢复 + PARAM_FORCE 注释区 + 20ms 链 IMU→Con_Task→Mode_Tick→Motor_Speed |
| Mode_1.h | ./Mode/Mode_1.h | 重写 | 调参模式头文件（导出 s_AT_Params/At_Size） |
| Mode_1.c | ./Mode/Mode_1.c | 重写 | 15 项参数表 + Param_Register + Param_Loop |
| Mymain.c | ./Top/Mymain.c | 重写 | F4 结构两套 switch + last_saved 模式记忆，删无条件 OLED_Clear |
| AllHeader.h | ./Top/AllHeader.h | 修改 | 补 Encoder_Key/TJC_LCD/IMU/MyPID/Queue/AT 全套/Con_Task/Control/Menu_Param 头文件 |
| AllHeader.c | ./Top/AllHeader.c | 修改 | 新 Initial_ALL：OLED→IMU→Serial→Encoder→Con_Motor→Param_AT24C02→Flash→Timer_Counter |
| Task.c | ./Software/Task.c | 删除 | 旧任务库已弃用（Con_Task 替代） |
| Task.h | ./Software/Task.h | 删除 | 旧任务库已弃用 |
| 引脚分配计划.md | ./引脚分配计划.md | 修改 | KEY3=PB0、Buzzer PE8 留档、新增 TIM7/16/17 定时器表 |
| F407移植计划.md | ./F407移植计划.md | 修改 | 17 步全部标记 ✅ |

## 2026-08-14 12:31 | Menu_Param 加入 Mode_3 + 仿照 F4 新增 7 个电机/直行调参项

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Motor.h | ./Hardware/Motor.h | 修改 | 新增 Wheel_Cm/PID_Pos/Angle_Ring_Enable 字段、Motor_Pos_Update 声明、Motor_Init 增 PID_Pos 第 9 参 |
| Motor.c | ./Hardware/Motor.c | 修改 | Motor_Init 初始化 PID_Pos/Angle_Ring_Enable(默认1)；新增 Motor_Pos_Update 位移计算 |
| Con_Motor.h | ./Function/Con_Motor.h | 修改 | 新增位置环 API（SetPos/Get_Pos/Is_Pos/Tick/Clear）与整车直行环声明 |
| Con_Motor.c | ./Function/Con_Motor.c | 修改 | PID_Pos 初始化 + Motor_Init 传 3 环；角度环加 Angle_Ring_Enable 开关；实现位置环与整车直行环（位置PD+梯形限速+yaw PD 差速） |
| MyEncoder.h | ./MySystem/MyEncoder.h | 修改 | 新增 MyEncoder_Total_Cnt_Clear 声明 |
| MyEncoder.c | ./MySystem/MyEncoder.c | 修改 | 新增 MyEncoder_Total_Cnt_Clear 实现（空指针保护） |
| Menu_Param.h | ./Function/Menu_Param.h | 修改 | 枚举扩至 9 项（新增 A/B 速度/角度/位置 + 整车直行）+ 21 个回调声明 |
| Menu_Param.c | ./Function/Menu_Param.c | 修改 | 新增 7 个调参任务实现（速度环关角度环/角度环由全局 20ms 链驱动/位置环 Dir=1/直行环）+ 标签表与任务表扩展 |
| Mode_3.c | ./Mode/Mode_3.c | 重写 | 改为 Menu_Task 模式（仿照 F4）：Setup=Menu_Tune_Init，Loop=Menu_Tune_Loop，Exit 停电机+恢复双电机角度环 |

## 2026-08-14 23:58 | 晾衣机器人业务（Mode_4）+ LCD 脱机阈值系统

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Robot_Task.h | ./Function/Robot_Task.h | 新增 | 晾衣任务系统头：私有任务枚举(4)、舵机角色宏映射、11 个命名阈值 extern、任务回调与业务 API |
| Robot_Task.c | ./Function/Robot_Task.c | 新增 | 任务实现：阈值默认值(V2实测)、任务表、晾衣9步/复位5步序列、急停/软停/超时保护、示教运动、ABC 命令解析(Rel/Abs/Save/Timeout/Hang_Go/Reset/Stop) |
| AllHeader.h | ./Top/AllHeader.h | 修改 | 加入 Robot_Task.h include |
| Mode_1.c | ./Mode/Mode_1.c | 修改 | s_AT_Params 追加 11 条 I32 晾衣阈值（地址自动分配 25~68） |
| Mode_G.c | ./Mode/Mode_G.c | 修改 | PARAM_FORCE 注释块追加 11 行晾衣阈值默认值示例 |
| TJC_LCD.h | ./Hardware/TJC_LCD.h | 修改 | 追加 TJC_LCD_Send_Text/Send_Num 声明（MCU→LCD 文本/数值发送） |
| TJC_LCD.c | ./Hardware/TJC_LCD.c | 修改 | 实现 Send_Text/Send_Num（TJC 原生命令 \xFF\xFF\xFF 终止，仿 Wave_Send_Float） |
| Mode_4.c | ./Mode/Mode_4.c | 重写 | 晾衣主模式状态机（IDLE/RUNNING/DONE/ERROR）+ 按键/LCD 触发 + OLED 渲染 + Tick 200ms LCD 回显 + Exit 软停兜底 |

## 2026-08-15 00:06 | Mode_4 OLED 显示去中文并下移到 20 行之后

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Mode_4.c | ./Mode/Mode_4.c | 修改 | OLED 状态/提示文案全部改英文，状态行从 y=10 移到 y=30（y=20 角度行之后），提示行保持 y=40 |

## 2026-08-15 00:21 | 任务原语并入全局 Task_Type，删除 Robot_Task 业务层（用户自行组装逻辑）

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Con_Task.h | ./Function/Con_Task.h | 修改 | 全局枚举 TASK_COUNT 前追加 TASK_SERVO_SET / TASK_CLEAR_ZERO / TASK_DONE |
| Control.h | ./Function/Control.h | 修改 | 追加舵机角色宏(SERVO_CLAW_A/B、SERVO_HANGER_1/2)+角色索引宏+3 组任务回调声明 |
| Control.c | ./Function/Control.c | 修改 | Control_TaskTable 注册 3 个新任务；实现 ServoSet/ClearZero/Done 回调（含 s_ServoMap） |
| AllHeader.h | ./Top/AllHeader.h | 修改 | 移除 Robot_Task.h include |
| Mode_1.c | ./Mode/Mode_1.c | 修改 | 移除 11 条晾衣阈值 AT 参数（恢复原 7 条） |
| Mode_G.c | ./Mode/Mode_G.c | 修改 | 移除 11 行晾衣阈值 PARAM_FORCE 注释示例 |
| Mode_4.c | ./Mode/Mode_4.c | 重写 | 改为任务组装空壳：Setup 注册全局表、Loop 保留 LCD_KEY_4=Skip+Con_Task_Loop+OLED 角度回显，留组装注释位 |
| Robot_Task.h | ./Function/Robot_Task.h | 删除 | 任务系统并入全局 Con_Task/Control，业务逻辑由用户自行组装 |
| Robot_Task.c | ./Function/Robot_Task.c | 删除 | 同上 |
| 工程小结.md | ./工程小结.md | 修改 | Task_Type 枚举列表同步 3 个新任务 |

## 2026-08-15 00:32 | 恢复 Robot_Task 任务系统与 11 条阈值（撤销上轮删除，仅 Mode_4 留组装空壳）

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Robot_Task.h | ./Function/Robot_Task.h | 新增 | 恢复：晾衣任务系统头（私有任务枚举、舵机角色宏、11 阈值 extern、业务 API） |
| Robot_Task.c | ./Function/Robot_Task.c | 新增 | 恢复：任务实现（序列、命令解析、超时/急停/软停、示教运动） |
| Mode_1.c | ./Mode/Mode_1.c | 修改 | 恢复 11 条晾衣阈值 AT 参数（Th_* 持久化） |
| Mode_G.c | ./Mode/Mode_G.c | 修改 | 恢复 11 行 PARAM_FORCE 阈值注释示例 |
| AllHeader.h | ./Top/AllHeader.h | 修改 | 恢复 Robot_Task.h include |
| 工程小结.md | ./工程小结.md | 修改 | Task_Type 枚举列表恢复原状（撤销新增任务项） |
| Mode_4.c | ./Mode/Mode_4.c | 修改 | 空壳改为对接 Robot_Task 私有任务系统（Setup=Robot_Task_Init），组装逻辑由用户编写 |

注：Con_Task.h / Control.h / Control.c 由用户手动恢复原状（未合并全局枚举），本记录不再重复。

## 2026-08-15 00:44 | 任务原语并入全局任务表 + Robot 库瘦身 + Mode_5 单任务测试（按已批准计划实施）

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Con_Task.h | ./Function/Con_Task.h | 修改 | 全局枚举 TASK_COUNT 前追加 TASK_MOTOR_TO（无超时）/TASK_SERVO_SET |
| Control.h | ./Function/Control.h | 修改 | 舵机角色宏+角色索引宏迁入；追加 Task_Motor_To / Task_Servo_Set 回调声明 |
| Control.c | ./Function/Control.c | 修改 | Control_TaskTable 注册 2 个新任务；s_ServoMap + 4 个回调实现 |
| Robot_Task.h | ./Function/Robot_Task.h | 修改 | 瘦身：删私有任务枚举/回调声明/超时/Abs-Rel 声明，只留 10 阈值+保持时长常量+错误码+业务 API |
| Robot_Task.c | ./Function/Robot_Task.c | 修改 | 瘦身：删私有任务表/示教运动函数/超时死代码；序列（晾衣8步/复位5步）与 12 条运动命令改走全局任务 |
| Mode_1.c | ./Mode/Mode_1.c | 修改 | 删除 Th_Motor_Timeout_ms AT 条目（剩 10 条阈值） |
| Mode_G.c | ./Mode/Mode_G.c | 修改 | 删除 Th_Motor_Timeout_ms PARAM_FORCE 注释行 |
| Mode_5.c | ./Mode/Mode_5.c | 重写 | 单任务测试：K1=电机A/K2=电机B 转测试角度，OLED 只显示参数（TgtA/TgtB/Tol/当前角度） |
| 工程小结.md | ./工程小结.md | 修改 | Task_Type 枚举列表同步 TASK_MOTOR_TO, TASK_SERVO_SET |

## 2026-08-15 02:54 | 新增双夹爪同步任务 TASK_CLAW_SET（AB 同时闭合/张开）

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Con_Task.h | ./Function/Con_Task.h | 修改 | 枚举 TASK_COUNT 前追加 TASK_CLAW_SET（p0=夹爪A角度, p1=夹爪B角度, p2=保持ms） |
| Control.h | ./Function/Control.h | 修改 | 追加 Task_Claw_Set_Setup/IsExit 声明 |
| Control.c | ./Function/Control.c | 修改 | Control_TaskTable 注册 TASK_CLAW_SET；回调实现：Setup 同帧同时设 A/B 两舵机角度，IsExit 按保持时间退出 |
| Robot_Task.c | ./Function/Robot_Task.c | 修改 | 晾衣②夹爪闭合、晾衣⑤夹爪张开、复位②夹爪张开 3 处由两行 SERVO_SET 串行改为一行 CLAW_SET 同步 |
| Mode_5.c | ./Mode/Mode_5.c | 修改 | LCD_KEY_1/2 夹爪测试同步改为 TASK_CLAW_SET 一行入队（用户自写测试程序） |
| 工程小结.md | ./工程小结.md | 修改 | Task_Type 枚举列表同步 TASK_CLAW_SET |

## 2026-08-15 03:51 | 重写 Robot_Cmd_Handle（新 LCD 协议）+ Sigan→Trans 命名统一

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Robot_Task.c | ./Function/Robot_Task.c | 修改 | 重写 Robot_Cmd_Handle：Hanger_Start/Back 业务触发（Start 忙时忽略）、11 条 Save 示教、7 条运动命令（帧内 = 分隔）；恢复 s_last_trans_rel 静态 |
| Robot_Task.h | ./Function/Robot_Task.h | 修改 | Th_Sigan_Step→Th_Trans_Step；补 Robot_Cmd_Handle 声明 |
| Mode_1.c | ./Mode/Mode_1.c | 修改 | AT 参数与 Param_Register 的 Th_Sigan_Step→Th_Trans_Step（显示名 Trans_Step） |
| Mode_G.c | ./Mode/Mode_G.c | 修改 | PARAM_FORCE 注释 Th_Sigan_Step→Th_Trans_Step |
| Mode_5.c | ./Mode/Mode_5.c | 修改 | K1 测试入队 Th_Sigan_Step→Th_Trans_Step |
| Con_Motor.h | ./Function/Con_Motor.h | 修改 | 死宏 Motor_Sigan_Next_Cnt→Motor_Trans_Next_Cnt |

## 2026-08-15 03:54 | 运动命令改为直接执行不入队（滑条实时重定目标）

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Robot_Task.c | ./Function/Robot_Task.c | 修改 | 7 条运动命令（Trans/Hanger Rel/Abs、ClawA/B、Hanger1）由 Con_Task_Enqueue 改为直接 Motor_SetAngle/Servo_SetAngle，去掉忙时丢弃判断，无需等待队列 |

## 2026-08-15 04:09 | 新增 6 条舵机到位命令（直接执行，目标=已存阈值）

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Robot_Task.c | ./Function/Robot_Task.c | 修改 | Robot_Cmd_Handle 新增 ClawA_Open/Close、ClawB_Open/Close、Hanger1_Open/Close 6 条 strcmp 匹配，直接 Servo_SetAngle 到 Th_* 阈值，不入队 |

## 2026-08-15 05:10 | Car 工程 Serial 库替换（新库：Serial_base + Serial_porting）

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Serial_base.h | D:/github/2-2-STM32/STM32/Projects/Robot2026/Sheng/Car/Hardware/Serial_base.h | 修改 | 用当前工程新 Serial 库覆盖（协议定义、Serial_Typedef 数据、错误码枚举），转 GBK 与 Car 工程一致 |
| Serial_base.c | D:/github/2-2-STM32/STM32/Projects/Robot2026/Sheng/Car/Hardware/Serial_base.c | 修改 | 协议常量实例 + 无参 Init（新 API），转 GBK |
| Serial_porting.h | D:/github/2-2-STM32/STM32/Projects/Robot2026/Sheng/Car/Function/Serial_porting.h | 新增 | 新外设层：Serial1/2/3 实例、DMA/IT 自适应收发；Serial4 已禁用（Car 无 UART4），转 GBK |
| Serial_porting.c | D:/github/2-2-STM32/STM32/Projects/Robot2026/Sheng/Car/Function/Serial_porting.c | 新增 | 新实现：Idle 中断统一入口、HEX/ABC 解析、发送 API、D-Cache 维护，转 GBK |
| Serial.h | D:/github/2-2-STM32/STM32/Projects/Robot2026/Sheng/Car/Function/Serial.h | 删除 | 旧库头文件，被 Serial_porting.h 取代（git 已跟踪可恢复） |
| Serial.c | D:/github/2-2-STM32/STM32/Projects/Robot2026/Sheng/Car/Function/Serial.c | 删除 | 旧库实现（旧 HAL_UARTEx_RxEventCallback 等），被 Serial_porting.c 取代（git 已跟踪可恢复） |
| AllHeader.h | D:/github/2-2-STM32/STM32/Projects/Robot2026/Sheng/Car/Top/AllHeader.h | 修改 | include "Serial.h" → "Serial_porting.h"（字节级替换，保留 GBK 编码） |
| Template.uvprojx | D:/github/2-2-STM32/STM32/Projects/Robot2026/Sheng/Car/MDK-ARM/Template.uvprojx | 修改 | Function 组文件条目 Serial.c/h → Serial_porting.c/h |

## 2026-08-15 06:58 | 新增 Mode_6：晾衣后双击 KEY_1 或 LCD Hanger_Shou 收衣服（复位）

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Robot_Task.h | ./Function/Robot_Task.h | 修改 | 业务 API 区新增 Robot_Shou_Start() 声明（收衣服，当前=复位序列） |
| Robot_Task.c | ./Function/Robot_Task.c | 修改 | 新增 Robot_Shou_Start() 实现（封装复位序列，后续可独立扩展）；Robot_Cmd_Handle 新增 Hanger_Shou 命令（任何时刻可用） |
| Mode_6.c | ./Mode/Mode_6.c | 修改 | 填充空模板：单击 KEY_1=晾衣、双击 KEY_1=收衣服、LCD 命令（Robot_Cmd_Handle）、Con_Task_Loop |
| 工程小结.md | ./工程小结.md | 修改 | Mode 现状表更新：Mode_4/5/6 已填充业务；待办清单勾选该项 |

## 2026-08-15 07:02 | 收衣服序列改为独立三步：①传送带回原位 ②丝杆下移 ③松夹爪

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Robot_Task.c | ./Function/Robot_Task.c | 修改 | Robot_Shou_Start() 不再复用复位序列，改为独立三步：①传送带回0 ②丝杆下移到底（Th_Hanger_Down）③松开夹爪 |
| Robot_Task.h | ./Function/Robot_Task.h | 修改 | Robot_Shou_Start 声明注释更新为新三步序列 |
| Mode_6.c | ./Mode/Mode_6.c | 修改 | 注释更正：双击=收衣服（去掉"复位"字样） |
| 工程小结.md | ./工程小结.md | 修改 | Mode_6 行收衣服序列描述更新 |

## 2026-08-15 08:03 | 修复 OLED_Data.c 编码错误（UTF-8 → GBK）

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| OLED_Data.c | ./Hardware/OLED_Data.c | 修改 | 文件被保存成 UTF-8，ARMCC v5 按 GBK 解析中文字符串时吃掉引号（missing closing quote）；转回 GBK 编码，内容未变，armcc 实测编译通过 |

## 2026-08-16 09:17 | 收衣服序列末尾新增 Car_Back 倒车指令（机械最后发帧 + 小车倒车状态）

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Con_Task.h | ./Function/Con_Task.h | 修改 | 任务枚举新增 TASK_SERIAL_CAR_BACK（串口发帧任务，收衣服序列最后一步） |
| Control.c | ./Function/Control.c | 修改 | 新增 Task_Serial_CarBack_Setup/IsExit（Serial3 发 @Car_Back$#，Setup 即发、立即 Exit）+ 任务表注册 |
| Control.h | ./Function/Control.h | 修改 | 新增任务 9 声明（Task_Serial_CarBack_Setup/IsExit） |
| Robot_Task.c | ./Function/Robot_Task.c | 修改 | Robot_Shou_Start：移除顶部发帧行，序列末尾入队 TASK_SERIAL_CAR_BACK（④ 接完衣服才发，防小车提前跑） |
| Con_Wheel_Control.h | D:\github\2-2-STM32\STM32\Projects\Robot2026\Sheng\Car\Function\Con_Wheel_Control.h | 修改 | 小车侧：枚举新增 Car_Turn_B（直线倒车） |
| Con_Wheel_Control.c | D:\github\2-2-STM32\STM32\Projects\Robot2026\Sheng\Car\Function\Con_Wheel_Control.c | 修改 | 小车侧：新增 Car_Turn_B_Setup/Tick/Is_Exit（目标 -50cm）+ Tick/Setup switch case + Car_Control_Change 退出分支 |
| Mode_5.c | D:\github\2-2-STM32\STM32\Projects\Robot2026\Sheng\Car\Mode\Mode_5.c | 修改 | 小车侧：新增 Car_Back 命令接收 → next_Status = Car_Turn_B |
