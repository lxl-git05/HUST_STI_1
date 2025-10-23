# Key

## 1. 简单说明

只设定了四个按键(大概率够了),需要几个就去.c文件的`Key_GetState`注释打开几个即可

==**建议Cube配置Key时使用标签如KEY1,KEY2(大写+数字)**==,因为程序是这么写的

## 2. 功能

* 单击 双击 长按 , 以后写OLED菜单很好用,调参也不错

* 取消双击功能:由于有双击逻辑,所以单击反应迟缓,如果不需要双击功能,则移步到.c将

```c
#define KEY_TIME_DOUBLE			200
```

改为

```c
#define KEY_TIME_DOUBLE			0
```

即可快速反应单击

## 3. 基础程序配置

* ```c
  #include "Key.h"
  ```

* setup: 无

* while: 无

* 中断(如`systick`,1ms一次)

```c
Key_Tick() ;// 更新状态,必备
```



例程:

```c
// 在while:
if (Key_Check(KEY_1 , KEY_SINGLE))
{
    HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin) ;
}
```





