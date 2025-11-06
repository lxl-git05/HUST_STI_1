# 2025 STM32 主程序

## 1. 资源分配

* 一般功能引脚

| 引脚号 |   标签   |      备注       |
| :----: | :------: | :-------------: |
|  PC13  |   LED0   |     板载LED     |
|  PB8   | OLED_SDA | 第一次PCB接反了 |
|  PB9   | OLED_SCL | 第一次PCB接反了 |
|  PB12  |   KEY1   |      按键1      |
|  PB13  |   KEY2   |      按键2      |

* 电机与编码器引脚

| 引脚号 |    标签     |      备注       |
| :----: | :---------: | :-------------: |
|  PA8   | Motor_A_IN1 |   电机A的PWM    |
|  PA11  | Motor_B_IN1 |   电机B的PWM    |
|  PB14  | Motor_A_IN2 | 电机A的普通引脚 |
|  PB15  | Motor_B_IN2 | 电机B的普通引脚 |
|  PA0   |  TIM2_CH1   | 编码器2的通道1  |
|  PA1   |  TIM2_CH2   | 编码器2的通道2  |
|  PA6   |  TIM3_CH1   | 编码器3的通道1  |
|  PA7   |  TIM3_CH2   | 编码器3的通道2  |

* 串口

| 引脚号 |   标签    | 备注 |
| :----: | :-------: | :--: |
|  PA9   | USART1_TX |      |
|  PA10  | USART1_RX |      |
|  PA2   | USART2_TX |      |
|  PA3   | USART2_RX |      |
|  PB10  | USART3_TX |      |
|  PB11  | USART3_RX |      |

* 其他

| 引脚号 | 标签 | 备注 |
| :----: | :--: | :--: |
|        |      |      |
|        |      |      |
|        |      |      |
|        |      |      |
|        |      |      |
|        |      |      |
|        |      |      |
|        |      |      |



1. USART1- PA9 , PA10
2. USART2 - PA2,PA3
3. TIM1 PWM_CH1 + CH4 PSC = 72-1 ARR = 100 - 1 
4. TIM2 Encoder
5. TIM3 Encoder

6. Motor_A_In2 : PB14 Motor_B_In2 : PB15

7. OLED PA8 PA9

## 2. 关于PID

​	真实PWM达到90+之后会有速度剧烈抖动,难以调控,所以建议进行限制,PWM达到90就别再上去了

但是同时电池电量降低时,相同的转速,PWM会增大,所以电池最好准备两块,保持高电量

## 3. 蓝牙通信

USART1已经被Serial接管,数据接收只能以一定的协议进行

电脑端为上位机,通过VOFA发送指令(HEX和文本两种类型)
STM32段为下位机,通过USART1接受指令,执行相应命令,但是STM32发送个电脑(VOFA)的信息不需要遵循相关协议

**使用USART1与电脑通信:**
VOFA改为115200 Serial改为1 Printf重定向为1
**使用蓝牙与电脑通信:**
搁置USART1 VOFA改为9600 Serial改为2 Printf重定向为2 