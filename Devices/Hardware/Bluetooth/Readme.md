# 蓝牙模块

## 1. Serial通信协议库

蓝牙通信需要满足该协议

详见Serial数据包通信-说明书



## 2. 蓝牙连接

详见蓝牙连接参考书



## 3. 蓝牙+VOFA动态调参

0. **Serial->STM32端==接收信息包==,使用哪一个串口接收信息就使用哪一个USART,自己去宏定义改**

   **printf->STM32端==发送信息==,使用哪一个串口发送就printf重定向为哪一个IDX**

1. 蓝牙可以使用USART2,那么Serial的USART就需要改为:

```c
#define Serial_huart huart2
#define Serial_USART USART2
```

2. 开启printf重定向,**先把usart.c的定义USART索引枚举打开!!!**

==**记住蓝牙需要9600波特率,所以USART2需要配置为9600!!!**==(USART1仍然是115200,毕竟使用场景是这样的)

3. 使用场景:

* 不使用蓝牙:
  * STM32直接与电脑通过USART1通信,波特率为115200,printf定向为IDX1,两者直接就这么通信
* 使用蓝牙:
  * 电脑使用VOFA,接入一个蓝牙(9600),开始与STM32的另一个蓝牙(9600)进行通信,STM32端为了给电脑蓝牙发消息,需要printf定向为IDX2,因此实现了通信

外设:

* 常用外设:OLED , KEY
* USART1 + USART2

