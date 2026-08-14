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

+++





# 2. 加入业务逻辑

现在开始我要加入业务逻辑：

你可以去到D:\github\2-2-STM32\STM32\Projects\Robot2026\XIAO\Robot_V2,关键的业务逻辑在于：

```c
void Control_Setup(void)
{
	// 初始化
	Queue_Init(&Hanger_Queue) ;
	// 入队
	Queue_Enqueue(&Hanger_Queue , Robot_Init) ;					// 初始化,丝杆在顶端, 夹子张开,传送带位置与衣架对应,衣架闭合
	Queue_Enqueue(&Hanger_Queue , Robot_Down) ;					// 电机向下够衣服
	Queue_Enqueue(&Hanger_Queue , Robot_Claw_Close) ;		// 夹衣服
	Queue_Enqueue(&Hanger_Queue , Robot_Mid) ;					// 电机上升到中位线
	Queue_Enqueue(&Hanger_Queue , Robot_Hanger1_Open) ;	// 衣架张开
	Queue_Enqueue(&Hanger_Queue , Robot_Claw_Open) ;		// 夹爪松开(张开)
	Queue_Enqueue(&Hanger_Queue , Robot_Up) ;						// 电机重新上升到顶点
	Queue_Enqueue(&Hanger_Queue , Robot_SiGan_Next) ;		// 传送带移动，整个过程完成1轮
	Queue_Enqueue(&Hanger_Queue , Robot_OK_1) ;					// 第1轮完成
	// 第2轮
	Queue_Enqueue(&Hanger_Queue , Robot_Down) ;					// 电机向下够衣服
	Queue_Enqueue(&Hanger_Queue , Robot_Claw_Close) ;		// 夹衣服
	Queue_Enqueue(&Hanger_Queue , Robot_Mid) ;					// 电机上升到中位线
	Queue_Enqueue(&Hanger_Queue , Robot_Hanger2_Open) ;	// 衣架2张开
	Queue_Enqueue(&Hanger_Queue , Robot_Claw_Open) ;		// 夹爪松开(张开)
	Queue_Enqueue(&Hanger_Queue , Robot_Up) ;						// 电机重新上升到顶点
	Queue_Enqueue(&Hanger_Queue , Robot_SiGan_Next) ;		// 传送带移动，整个过程再次完成1轮
	Queue_Enqueue(&Hanger_Queue , Robot_OK_2) ;					// 第2轮完成
}
```

而当前我本工程已经有了架构更加清晰的任务系统和脱机阈值+LCD调试系统，那么我应该如何重构上面的逻辑呢：我目前希望的是实现以下功能：

+ 功能的调用暂时写在Mode4

+ [ ] 晾衣服（也就是第一次晾衣服，第2次都没必要了，第1次以上的晾衣服我后续再开发，你先不用管）
+ [ ] 复位（也就是第1次晾衣服完成之后，如果不复位，后续还需要手动掰扯，所以需要复位，方便后续多次使用）
+ [ ] 你先帮我思考上面两个功能怎么拆解成任务，并且参数最好是不多，并且可以后续加入脱机阈值，当然我们都可以讨论
+ [ ] 上面两个是主要希望实现的功能，而下面则是脱机阈值：
  + [ ] 电机：
    + [ ] 我打算LCD端发送：`@Hanger_Rel:%d$# \ Sigan_Rel:%d$#`，那么当前电机就==相对==移动%d的值，这样就不需要断点再掰扯电机了，后续回位之后直接reset即可归零，这主要是应对突发情况不好进行回位的时候使用
    + [ ] 后续@等帧格式是有的，这里省略
    + [ ] LCD端发送：`@Hanger_Abs:%d$# \ Sigan_Abs:%d$# `，然后后续再发送`Save_x` x: `Hanger_Up\Mid\Down 、 SiGan_Next` 前者是保存绝对值，后者是保存每次++的时候移动的相对值，当然一般晾完一次衣服就回位了，你看看怎么搞更加简洁明了
  + [ ] 舵机：
    + [ ] 你根据电机进行同类思考
+ [ ] 上面只是我的想法，你需要自己独立思考，看看有没有更加简洁高效的方法，并且可拓展性更强，以你为主

开始plan吧——任务系统+LCD脱机调试





