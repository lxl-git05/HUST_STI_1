# 1. 项目移植	

 	现在开始我要基于当前目录进行工程的项目移植（从D:\github\HUST_STI\HUST_STI_1\Templete\Templete_STM32F407ZGT6）移植进来，实现更多的底层和逻辑功能，首先你需要做：

+ [ ] 使用ch skill，阅读本工程，了解本工程目前的硬件资源、架构和内容
+ [ ] 阅读D:\github\HUST_STI\HUST_STI_1\Templete\Templete_STM32F407ZGT6的工程内容，可以通过各个md了解整体概况，然后具体进入库函数进行简单验证，防止md没更新导致和事实不符
+ [ ] 参考引脚分配计划.md，检查是否有问题
+ [ ] 对比两工程的不同和相同，然后给出本工程的移植计划：分功能，一个库一个库的进行移植

+ [ ] 写移植计划的时候可以询问我细节问题，放置错误移植或者多余移植

+++

1. 蜂鸣器不需要移植
2. 可以先查看当前的MySystem的各个底层库是否能够新增函数，先完成底层的建立
3. 串口接收都使用中断接收中断，发送使用阻塞发送即可，串口的功能尽量参考F4的，不要再基于F4大改
4. AT相关的库进行新建文件夹：AT，就像IMU一样，这样结构更加清晰
5. Keil加入库路径我来实现即可，无需你操作keil
6. 现在开始进行移植计划表，写个移植简明内容即可，从1到x，我同意之后，你新建移植计划md和记忆，然后就开始一项一项移植

+++

 Menu_Param加入Mode3中，并且仿照F4，加入

​      TUNE_MOTOR_A_SPEED,      // Motor_A 速度环
​      TUNE_MOTOR_A_ANGLE,      // Motor_A 角度环
​      TUNE_MOTOR_A_POS,        // Motor_A 位置环
​      TUNE_MOTOR_B_SPEED,      // Motor_B 速度环
​      TUNE_MOTOR_B_ANGLE,      // Motor_B 角度环
​      TUNE_MOTOR_B_POS,        // Motor_B 位置环
​      TUNE_CAR_STRAIGHT,       // 整车直行环（仅1个）