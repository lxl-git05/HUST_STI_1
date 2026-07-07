

​	现在我要开始移植AT24C02,首先你需要建立软件IIC库，可以参考D:\github\HUST_STI\HUST_STI_1\Templete\Template_STM32H743VIT6\模板工程文件夹配置\AC外存\MyIIC，建立一个可以软件驱动任何IIC外设的软件库，然后再导入AT24C02代码，重构使用该库，那么首先你需要：

1. 建立MyI2C库放在MySystem，最终目的是在任何芯片都只需要修改本底层就能驱动任何IIC外设，那么你需要考虑写两套驱动，一套软件一套硬件IIC，当然了，STM32的硬件IIC已经很简洁了，所以你只需要套壳即可
2. 尝试驱动AT24C02底层，然后写测试代码进行底层测试
3. 最后移植AT24C02的顶层的四个文件，然后写测试代码进行测试

现在你先完成第1步，进行plan

+++


 Ready to code?

 Here is Claude's plan:
╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 Plan: MyI2C 软件/硬件抽象库（第1步）

 Context

 需要为 STM32H743VIT6 模板工程建立 MyI2C 抽象库，放在 MySystem/ 层。最终目标是：移植到任何芯片时，只需修改 MySystem
 层即可驱动任何 I2C 外设（AT24C02、其他传感器等）。

 已有参考：
 - 模板工程文件夹配置/AC外存/MyI2C/ — F103 目标的软I2C实现（delay用NOP循环，需适配H743主频）
 - 模板工程文件夹配置/AC外存/bsp_at24c02.c/h — AT24C02驱动（含软/硬I2C两套实现，SDA方向切换更完善）
 - Hardware/OLED.c — 已使用 MyGPIO 抽象的软件I2C，但write-only（无SDA读、无ACK检测）

 设计方案

 1. 模块结构

 MySystem/
 ├ MyI2C.h       // 统一头文件：MyI2C_Typedef结构体 + API声明 + 模式开关
 └ MyI2C.c       // 软I2C实现 + 硬I2C实现（均依赖MyGPIO抽象）

 2. MyI2C_Typedef 结构体

 typedef struct {
     MyGPIO_Typedef *scl;    // SCL GPIO（软件/硬件I2C均使用MyGPIO抽象）
     MyGPIO_Typedef *sda;    // SDA GPIO（仅软件I2C用；硬件I2C填NULL）
     uint32_t delay_us;      // 时钟延时（us，软件I2C用；硬件模式忽略）
 } MyI2C_Typedef;

 ▎ ==注意：SDA 在软件I2C中需要方向切换（输出→输入用于ACK检测），因此单独传入 MyGPIO 实例。硬件I2C模式下 sda 填 NULL。==

 3. 延时方案：DWT Cycle Counter（频率无关）

 采用 ARM CoreDebug DWT CYCCNT 计数器实现微秒延时，不依赖 NOP 循环，跨芯片频率自适应：

 // 延时 n 微秒，使用 DWT CYCCNT（需使能 DWT）
 static inline void MyI2C_DelayUs(uint32_t us) {
     uint32_t start = DWT->CYCCNT;
     uint32_t ticks = us * (SystemCoreClock / 1000000UL);
     while ((DWT->CYCCNT - start) < ticks);
 }

 需在 MyI2C_Init() 中调用 CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; DWT->CYCCNT = 0; DWT->CTRL |=
 DWT_CTRL_CYCCNTENA_Msk; 初始化 DWT。

 4. 软件I2C实现（MyI2C.c 静态函数）

 参考 bsp_at24c02.c 的更完善实现（相比 AC外存/MyI2C/MyI2C.c），包含：
 - SW_I2C_SDA_Mode_In() — SDA切换为输入（浮空）
 - SW_I2C_SDA_Mode_Out() — SDA切换为推挽输出
 - SW_I2C_Start() / SW_I2C_Stop()
 - SW_I2C_SendByte() / SW_I2C_ReceiveByte()
 - SW_I2C_SendAck() / SW_I2C_WaitAck() — 含超时检测（bsp_at24c02有，MyIIC参考缺失）

 所有GPIO操作经由 MyGPIO_WritePin(MyI2C->scl, ...) / MyGPIO_ReadPin(MyI2C->sda, ...) 抽象。

 5. 硬件I2C实现（MyI2C.c 静态函数）

 直接调用 HAL I2C API 作为 thin wrapper：

 static HAL_StatusTypeDef HW_I2C_Mem_Write(MyI2C_Typedef *i2c, uint16_t DevAddr, uint16_t MemAddr, uint16_t MemAddSize,
 uint8_t *pData, uint16_t Size);
 static HAL_StatusTypeDef HW_I2C_Mem_Read(MyI2C_Typedef *i2c, uint16_t DevAddr, uint16_t MemAddr, uint16_t MemAddSize,
 uint8_t *pData, uint16_t Size);

 内部使用 extern I2C_HandleTypeDef hi2c1;（CubeMX生成的全局句柄），直接转发 HAL I2C API：

 static HAL_StatusTypeDef HW_I2C_Mem_Write(uint16_t DevAddr, uint16_t MemAddr, uint8_t *pData, uint16_t Size) {
     return HAL_I2C_Mem_Write(&hi2c1, DevAddr, MemAddr, I2C_MEMADD_SIZE_8BIT, pData, Size, 100);
 }
 static HAL_StatusTypeDef HW_I2C_Mem_Read(uint16_t DevAddr, uint16_t MemAddr, uint8_t *pData, uint16_t Size) {
     return HAL_I2C_Mem_Read(&hi2c1, DevAddr, MemAddr, I2C_MEMADD_SIZE_8BIT, pData, Size, 100);
 }
 static HAL_StatusTypeDef HW_I2C_IsDeviceReady(uint16_t DevAddr, uint32_t Trials) {
     return HAL_I2C_IsDeviceReady(&hi2c1, DevAddr, Trials, 100);
 }

 6. **统一API（对外接口）**

 **// 初始化（使能DWT + 软/硬I2C GPIO配置）**
 **void MyI2C_Init(MyI2C_Typedef *i2c);**

 **// I2C事务（软硬共用）**
 **HAL_StatusTypeDef MyI2C_Mem_Write(MyI2C_Typedef *i2c, uint16_t DevAddr, uint16_t MemAddr, uint8_t *pData, uint16_t**
 **Size);**
 **HAL_StatusTypeDef MyI2C_Mem_Read(MyI2C_Typedef *i2c, uint16_t DevAddr, uint16_t MemAddr, uint8_t *pData, uint16_t**
 **Size);**
 **HAL_StatusTypeDef MyI2C_IsDeviceReady(MyI2C_Typedef *i2c, uint16_t DevAddr, uint32_t Trials);**

 7. 模式切换

 MyI2C.h 中通过 #define MYI2C_MODE SW_I2C 切换（SW_I2C = 0, HW_I2C = 1）。所有 API 函数内部用 #if MYI2C_MODE == SW_I2C
 分支调用软或硬实现。

 需要创建/修改的文件

 ┌┬┬┐
 │        文件         │ 操作 │                                      说明                                       │
 ├┼┼┤
 │ MySystem/MyI2C.h    │ 新增 │ 结构体定义 + API声明 + 模式开关（SW_I2C/HW_I2C）                                │
 ├┼┼┤
 │ MySystem/MyI2C.c    │ 新增 │ 软I2C + 硬I2C（thin wrapper）实现                                               │
 ├┼┼┤
 │ MySystem/MySystem.h │ 修改 │ 添加 #include "MyI2C.h"                                                         │
 ├┼┼┤
 │ MySystem/MySystem.c │ 修改 │ 添加 MyI2C_Typedef MyI2C_Bus = { &MyGPIO_OLED_SCL, &MyGPIO_OLED_SDA, ... } 实例 │
 └┴┴┘

 ▎ 注：AT24C02 和 OLED 共用同一组物理引脚和 I2C 总线，所以 MyI2C_Bus 实例直接复用 MyGPIO_OLED_SCL/SDA，不需要额外定义新
 ▎ GPIO 实例。

 HAL_StatusTypeDef MyI2C_Mem_Read(MyI2C_Typedef *i2c, uint16_t DevAddr, uint16_t MemAddr, uint8_t *pData, uint16_t Size);
 HAL_StatusTypeDef MyI2C_IsDeviceReady(MyI2C_Typedef *i2c, uint16_t DevAddr, uint32_t Trials);

 7. 模式切换

 MyI2C.h 中通过 #define MYI2C_MODE SW_I2C 切换（SW_I2C = 0, HW_I2C = 1）。所有 API 函数内部用 #if MYI2C_MODE == SW_I2C 分支调用软或硬实现。

 需要创建/修改的文件

 ┌┬┬┐
 │        文件         │ 操作 │                                      说明                                       │
 ├┼┼┤
 │ MySystem/MyI2C.h    │ 新增 │ 结构体定义 + API声明 + 模式开关（SW_I2C/HW_I2C）                                │
 ├┼┼┤
 │ MySystem/MyI2C.c    │ 新增 │ 软I2C + 硬I2C（thin wrapper）实现                                               │
 ├┼┼┤
 │ MySystem/MySystem.h │ 修改 │ 添加 #include "MyI2C.h"                                                         │
 ├┼┼┤
 │ MySystem/MySystem.c │ 修改 │ 添加 MyI2C_Typedef MyI2C_Bus = { &MyGPIO_OLED_SCL, &MyGPIO_OLED_SDA, ... } 实例 │
 └┴┴┘

 ▎ 注：AT24C02 和 OLED 共用同一组物理引脚和 I2C 总线，所以 MyI2C_Bus 实例直接复用 MyGPIO_OLED_SCL/SDA，不需要额外定义新 GPIO 实例。

 │ MySystem/MyI2C.h    │ 新增 │ 结构体定义 + API声明 + 模式开关（SW_I2C/HW_I2C）                                │
 ├┼┼┤
 │ MySystem/MyI2C.c    │ 新增 │ 软I2C + 硬I2C（thin wrapper）实现                                               │
 ├┼┼┤
 │ MySystem/MySystem.h │ 修改 │ 添加 #include "MyI2C.h"                                                         │
 ├┼┼┤
 │ MySystem/MySystem.c │ 修改 │ 添加 MyI2C_Typedef MyI2C_Bus = { &MyGPIO_OLED_SCL, &MyGPIO_OLED_SDA, ... } 实例 │
 └┴┴┘

 ▎ 注：AT24C02 和 OLED 共用同一组物理引脚和 I2C 总线，所以 MyI2C_Bus 实例直接复用 MyGPIO_OLED_SCL/SDA，不需要额外定义新
 ▎ GPIO 实例。

 HAL_StatusTypeDef MyI2C_Mem_Read(MyI2C_Typedef *i2c, uint16_t DevAddr, uint16_t MemAddr, uint8_t *pData, uint16_t Size);
 HAL_StatusTypeDef MyI2C_IsDeviceReady(MyI2C_Typedef *i2c, uint16_t DevAddr, uint32_t Trials);

 7. 模式切换

 MyI2C.h 中通过 #define MYI2C_MODE SW_I2C 切换（SW_I2C = 0, HW_I2C = 1）。所有 API 函数内部用 #if MYI2C_MODE == SW_I2C 分支调用软或硬实现。

 需要创建/修改的文件

 ┌┬┬┐
 │        文件         │ 操作 │                                      说明                                       │
 ├┼┼┤
 │ MySystem/MyI2C.h    │ 新增 │ 结构体定义 + API声明 + 模式开关（SW_I2C/HW_I2C）                                │
 ├┼┼┤
 │ MySystem/MyI2C.c    │ 新增 │ 软I2C + 硬I2C（thin wrapper）实现                                               │
 ├┼┼┤
 │ MySystem/MySystem.h │ 修改 │ 添加 #include "MyI2C.h"                                                         │
 ├┼┼┤
 │ MySystem/MySystem.c │ 修改 │ 添加 MyI2C_Typedef MyI2C_Bus = { &MyGPIO_OLED_SCL, &MyGPIO_OLED_SDA, ... } 实例 │
 └┴┴┘

 ▎ 注：AT24C02 和 OLED 共用同一组物理引脚和 I2C 总线，所以 MyI2C_Bus 实例直接复用 MyGPIO_OLED_SCL/SDA，不需要额外定义新 GPIO 实例。

 ├┼┼┤
 │ MySystem/MyI2C.c    │ 新增 │ 软I2C + 硬I2C（thin wrapper）实现                                               │
 ├┼┼┤
 │ MySystem/MySystem.h │ 修改 │ 添加 #include "MyI2C.h"                                                         │
 ├┼┼┤
 │ MySystem/MySystem.c │ 修改 │ 添加 MyI2C_Typedef MyI2C_Bus = { &MyGPIO_OLED_SCL, &MyGPIO_OLED_SDA, ... } 实例 │
 └┴┴┘

 ▎ 注：AT24C02 和 OLED 共用同一组物理引脚和 I2C 总线，所以 MyI2C_Bus 实例直接复用 MyGPIO_OLED_SCL/SDA，不需要额外定义新
 ▎ GPIO 实例。

 HAL_StatusTypeDef MyI2C_Mem_Read(MyI2C_Typedef *i2c, uint16_t DevAddr, uint16_t MemAddr, uint8_t *pData, uint16_t Size);
 HAL_StatusTypeDef MyI2C_IsDeviceReady(MyI2C_Typedef *i2c, uint16_t DevAddr, uint32_t Trials);

 7. 模式切换

 MyI2C.h 中通过 #define MYI2C_MODE SW_I2C 切换（SW_I2C = 0, HW_I2C = 1）。所有 API 函数内部用 #if MYI2C_MODE == SW_I2C 分支调用软或硬实现。

 需要创建/修改的文件

 ┌┬┬┐
 │        文件         │ 操作 │                                      说明                                       │
 ├┼┼┤
 │ MySystem/MyI2C.h    │ 新增 │ 结构体定义 + API声明 + 模式开关（SW_I2C/HW_I2C）                                │
 ├┼┼┤
 │ MySystem/MyI2C.c    │ 新增 │ 软I2C + 硬I2C（thin wrapper）实现                                               │
 ├┼┼┤
 │ MySystem/MySystem.h │ 修改 │ 添加 #include "MyI2C.h"                                                         │
 ├┼┼┤
 │ MySystem/MySystem.c │ 修改 │ 添加 MyI2C_Typedef MyI2C_Bus = { &MyGPIO_OLED_SCL, &MyGPIO_OLED_SDA, ... } 实例 │
 └┴┴┘

 ▎ 注：AT24C02 和 OLED 共用同一组物理引脚和 I2C 总线，所以 MyI2C_Bus 实例直接复用 MyGPIO_OLED_SCL/SDA，不需要额外定义新 GPIO 实例。

 ├┼┼┤
 │ MySystem/MyI2C.c    │ 新增 │ 软I2C + 硬I2C（thin wrapper）实现                                               │
 ├┼┼┤
 │ MySystem/MySystem.h │ 修改 │ 添加 #include "MyI2C.h"                                                         │
 ├┼┼┤
 │ MySystem/MySystem.c │ 修改 │ 添加 MyI2C_Typedef MyI2C_Bus = { &MyGPIO_OLED_SCL, &MyGPIO_OLED_SDA, ... } 实例 │
 └┴┴┘

 ▎ 注：AT24C02 和 OLED 共用同一组物理引脚和 I2C 总线，所以 MyI2C_Bus 实例直接复用 MyGPIO_OLED_SCL/SDA，不需要额外定义新
 ▎ GPIO 实例。

 │ MySystem/MySystem.c │ 修改 │ 添加 MyI2C_Typedef MyI2C_Bus = { &MyGPIO_OLED_SCL, &MyGPIO_OLED_SDA, ... } 实例 │
 └┴┴┘

 ▎ 注：AT24C02 和 OLED 共用同一组物理引脚和 I2C 总线，所以 MyI2C_Bus 实例直接复用 MyGPIO_OLED_SCL/SDA，不需要额外定义新 GPIO
 ▎ 实例。

 AT24C02 / OLED 共用引脚确认

 OLED 和 AT24C02 共用同一组 I2C 总线（SCL/SDA），这是 I2C 的标准用法（多从机不同地址）。

 - SCL → OLED_SCL 引脚（PB6，已在 CubeMX 中定义 label）
 - SDA → OLED_SDA 引脚（PB7，已在 CubeMX 中定义 label）
 - 硬件 I2C 外设：hi2c1（已在 CubeMX 中配置）
 - AT24C02 7-bit 设备地址：0x50（0xA0 写 / 0xA1 读）

 ▎ 只需在 MySystem.c 中定义 MyI2C 实例指向 OLED_SCL/SDA 的 port/pin 即可，共用总线意味着任何一方的初始化不会影响另一方。

 验证方案

 1. 编译检查：无警告无错误
 2. 写入测试：向 AT24C02 地址 0 写入 0x5A，然后读回比对
 3. 边界测试：写入 0x00 确认 ACK 正常
 4. 完整读写循环：写一个已知 pattern（如 0x55），读回验证