## 2026-07-15 | 电机PID调参示例代码——用KEY_1单击切换电机替代#ifdef

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Mode_2.c | ./Template_F407ZGT6/Mode/Mode_2.c | 修改 | 电机PID调参实验模式：KEY_1单击切换选择Motor A/B，串口设置Kp/Ki/Kd/目标速度，Tick打印速度数据 |

## 2026-07-16 | 移植Mode_2到Mode_4 + 新建Mode_5/6 + 集成到Mymain

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Mode_4.c | ./Template_F407ZGT6/Mode/Mode_4.c | 修改 | 从Mode_2等效移植电机PID调参代码 |
| Mode_5.h | ./Template_F407ZGT6/Mode/Mode_5.h | 新增 | Mode_5 头文件，标准四函数声明 |
| Mode_5.c | ./Template_F407ZGT6/Mode/Mode_5.c | 新增 | Mode_5 实现，空壳模板 |
| Mode_6.h | ./Template_F407ZGT6/Mode/Mode_6.h | 新增 | Mode_6 头文件，标准四函数声明 |
| Mode_6.c | ./Template_F407ZGT6/Mode/Mode_6.c | 新增 | Mode_6 实现，空壳模板 |
| Mode_G.h | ./Template_F407ZGT6/Mode/Mode_G.h | 修改 | 枚举新增 Mode_5, Mode_6 |
| Mode_G.c | ./Template_F407ZGT6/Mode/Mode_G.c | 修改 | Timer_20ms_Callback 新增 case 5/6 |
| Mymain.c | ./Template_F407ZGT6/Top/Mymain.c | 修改 | Loop/Exit/Setup 三处 switch 新增 case 5/6 |
| AllHeader.h | ./Template_F407ZGT6/Top/AllHeader.h | 修改 | 新增 #include "Mode_5.h" "Mode_6.h" |
| Template.uvprojx | ./Template_F407ZGT6/MDK-ARM/Template.uvprojx | 修改 | Mode 组新增 Mode_5.c/.h, Mode_6.c/.h |

## 2026-07-16 | Mode_2清空→电机PID代码迁移到Con_Mode_1 + Con_Mode_1~6集成到Mymain框架

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Mode_2.c | ./Template_F407ZGT6/Mode/Mode_2.c | 修改 | 清空为原始框架（Setup/Loop/Tick/Exit空壳） |
| Con_Mode_1.h | ./Template_F407ZGT6/Con_Mode/Con_Mode_1.h | 修改 | 电机PID调参模块，四函数声明 |
| Con_Mode_1.c | ./Template_F407ZGT6/Con_Mode/Con_Mode_1.c | 修改 | 电机PID调参实现（从Mode_2迁移），KEY_1切换Motor A/B |
| Con_Mode_2~6.h | ./Template_F407ZGT6/Con_Mode/Con_Mode_2~6.h | 修改 | 统一加入四函数声明 |
| Con_Mode_2~6.c | ./Template_F407ZGT6/Con_Mode/Con_Mode_2~6.c | 修改 | 统一加入四函数空壳实现 |
| Mode_G.h | ./Template_F407ZGT6/Mode/Mode_G.h | 修改 | 枚举新增 Con_Mode_1~6 |
| Mode_G.c | ./Template_F407ZGT6/Mode/Mode_G.c | 修改 | Timer_20ms_Callback 新增 Con_Mode_1~6 的 Tick 分发 |
| Mymain.c | ./Template_F407ZGT6/Top/Mymain.c | 修改 | Loop/Exit/Setup 三处 switch 新增 Con_Mode_1~6 |
| AllHeader.h | ./Template_F407ZGT6/Top/AllHeader.h | 修改 | 新增 Con_Mode_1~6.h 的 include |
