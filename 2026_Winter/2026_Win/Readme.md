[toc]

# Readme

## 1. 介绍

这是一个F1的标准工程,加入了一些基本功能

建议日后加入库函数的逻辑:

* System : 系统的库函数 , 十分底层 , 如PWM , Encoder , ADC等
* Software: 相关算法
* Hardware: 与硬件有关的库
* Control:控制库,综合Software和Hardware的内容进行核心顶层代码的编写



### 1-1 OLED

​	使用软件IIC

​	这个功能简单易懂,自己看.h的函数介绍

**基础配置:**

```c
// 放在setup:
void OLED_Init(void);  // 初始化

// 放在while:
// 自己看介绍,最好用的函数之一
void OLED_Printf(int16_t X, int16_t Y, uint8_t FontSize, char *format, ...);	

void OLED_Clear(void);  // OLED屏有时候会出现数据滞留,建议在OLED展示前加上Clear,实现数据即时更新
void OLED_Update(void);	// 必备,刷新函数
```

### 1-2 Key

​	功能:实现按键单击双击和长按逻辑,双击逻辑可以通过改变宏定义取消(使得单击从延迟反应到立刻反应)

**基础配置:**

```c
// 放在setup:
无
// 放在while:
uint8_t Key_Check(uint8_t n, uint8_t Flag);	// 非阻塞实现按键读取
// 放在定时器(1ms)
void Key_Tick(void);
```

### 1-3 PID

​	全能版PID,实现大部分PID特殊化处理

**基础配置:**

```c
// 放在setup:
void PID_Init(Pid_Typedef *pid, float kp, float ki, float kd , float OutMax , float OutMin , float ioutMax ) ;	// 用来一般化初始化PID结构体

// 放在while:
无
// 放在定时器
// PID值更新,更新值直接写入PID的Output,记得放在定时器的10-100ms的周期中
void PID_Update(Pid_Typedef *pid, float ActualValue) ;
```

* 更多功能:需要自己配置,过于复杂所以没有封装函数
  * 输入死区
  * 微分先行
  * 不完全微分
  * `void (*PID_Func)(void);`针对性PID特殊处理,如变速积分等

### 1-4 Serial

​	串口,串口协议见software内的协议说明,此处略

**基础配置:**

```c
// 全局变量
extern Serial_HEX_Data_Typedef   Serial_Hex_Data ;			// 解析好的HEX数据包
extern Serial_ABC_Data_Typedef   Serial_ABC_Data ;			// 解析好的ABC数据包

// setup:
Serial_Init(&Serial_huart) ;	// *串口初始化*

// while:
去看库内的核心函数说明,这里是基础配置所以不展示
```

**功能实例:**

```c
// HEX
if (Serial_GetNewPackageFlag_HEX() == 1)
{
    // OLED展示各个数据
    OLED_ShowNum(0 , 20 , Serial_Hex_Data.Serial_New_Package[0] , 1 , OLED_8X16 ) ;
    for (int i = 1 ; i < Serial_Hex_Data.Serial_New_Package[0] + 1 ; i ++)
    {
        OLED_ShowNum(20 , 10 + 10 * i , Serial_Hex_Data.Serial_New_Package[i] , 5 , OLED_6X8 ) ;
    }
}
// ABC
if (Serial_GetNewPackageFlag_ABC() == 1)
{
    // 文本包调试程序
    Serial_SetFloatData("Kp" , "Kp=%f" , &kp) ;
    Serial_SetFloatData("Ki" , "Ki=%f" , &ki) ;
    Serial_SetFloatData("Kd" , "Kd=%f" , &kd) ;

    // OLED展示
}
```

### 1-5 `Mymain`

​	截胡主函数 , 减少杂七杂八的main的注释,.h则是#include了很多东西



### 1-6 Timer_Counter

​	计时函数,用来计算某些语句或者某个函数的时间消耗状况

**基础配置:**

```c
// setup:
// 计时器初始化
void Timer_Counter_Init(void);

// while:
无
```

* 第一种:

```c
extern float time_us ;				// 代码之间的时间间隔
// 计时器开始计时
void Timer_Counter_Begin(void);

// 被计算时间消耗的一些语句

// 计时器结束计时
void Timer_Counter_End(void);
```

* 第二种:

```c
extern float time_Func_us ;		// 函数两次执行的时间间隔
// 计算一个多次执行的函数的每次执行间隔时间,放在函数中即可
void Timer_Counter_Func(void) ;
```



### 1-7 Task

​	任务执行函数,用来进行等频率任务调度

**基础配置:**

```c
// setup:
mytask 任务名称 ;

void taskInit(mytask* task,uint32_t cnt_init,uint32_t cycle_init ,void (*callback_func)(void) );		   // 任务初始化(setup),回调函数记得写

// while:
无

// 定时器(1ms)
void task_possess(mytask* task);	// 任务周期函数(放在定时器)
```



### 1-8 PWM 

​	PWM的基础底层代码

**基础配置:**

```c
// setup:
// PWM初始化
void PWM_Init(TIM_HandleTypeDef htimx , uint32_t Channel) ;

// while:
// 设置PWM值
void PWM_SetCompare1(TIM_HandleTypeDef htimx , uint32_t Channel , uint16_t Compare) ;
```



### 1-9 Encoder

​	编码器的底层代码

**基础配置:**

```c
// setup:
// 编码器初始化
void Encoder_Init(TIM_HandleTypeDef *htimx) ;

// 定时器
// 得到编码器的脉冲数,需要结合固定时间来计算"速度"
int Encoder_Get_CNT(TIM_HandleTypeDef *htimx) ;
```



## 2. 引脚

### 2-1 一般功能引脚

| 引脚号 |   标签   |    备注     |
| :----: | :------: | :---------: |
|  PC13  |   LED0   | 内部板载LED |
|  PB8   | OLED_SCL |             |
|  PB9   | OLED_SDA |             |
|  PB12  |   KEY1   |    按键1    |
|  PB13  |   KEY2   |    按键2    |

### 2-2 串口

| 引脚号 |   标签    | 备注 |
| :----: | :-------: | :--: |
|  PA9   | USART1_TX |      |
|  PA10  | USART1_RX |      |