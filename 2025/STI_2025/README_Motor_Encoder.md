# 编码器测速+PID调节电机程序

## 1. 简要说明

### 1-1 代码

​	由于我只有一个电机,所以只引入了Motor_A,Motor_B由于资源冲突懒得解决导致连编码器都没有,所以建议以后有需要再进行重构

​	**要点**:

* 程序中先关闭了`Systick`再打开,是因为电机初始化还没完成时就进入了Sys回调函数,开始运行PID和测速,导致速度配置的时候引脚还未初始化,导致空指针越界,进入`HardFault_Handler`中断循环,所以先关闭了`Systick`再打开
* 电机的结构体与PID结构体有一个参数冲突,就是`GoalPoint`(目标速度),所以我在Motor_PID_Tick中加入了,以后有机会重构的时候再解决

```c
// 更新一下.路径冲突了,怕出问题
Motor->PID.goalPoint = Motor->Motor_GoalSpeed ;
```

* 关于得到脉冲数:

当时搞了好久,发现使用`cnt = TIM2->CNT`可行,但是`cnt = __HAL_TIM_GET_COUNTER(&Motor->Motor_Encoder_htim);`不可行,最后发现原来是`Motor->Motor_Encoder_htim`指向了定时中断的`htim`导致数据不正确,这里要注意一下