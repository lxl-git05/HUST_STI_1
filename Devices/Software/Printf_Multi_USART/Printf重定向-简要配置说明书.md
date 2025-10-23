[toc]

##  ==printf重映射==

## 1-1 简单的单串口printf

* cube配置,**记得打开中断**就行,否则就会卡死在while

* 第一步

在main.c:

```c
#include <stdio.h>
```

* 第2步

在main.c

```c
// 串口printf重定向
int fputc(int ch , FILE *f)
{
	HAL_UART_Transmit_IT(&huart1, (uint8_t *)&ch, 1);
	// 检查UART是否就绪,如果不等待肯定出问题
	while (huart1.gState != HAL_UART_STATE_READY) ;
	return ch ;
}
```

* 第3步

魔术棒->**勾上Use MicroLIB即可**

==**建议:不要把printf和DMA放在一起(资源占用相同,一起起作用会冲突,所以建议printf作调试用,DMA在正式工程使用)**==

当然,再开一个USART就可以解决这个问题了,专门给printf,防止资源冲突

## 1-2 ==多路USART串口printf==

* **①** Cube配置:**打开USART就行,其他的都不需要配置**
* **②**==**勾上Use MicroLIB即可**==

* **③** 在==usart.c:==

**首先:**

```c
#include <stdio.h>
```

**其次:**

注:开了几个USART就取消注释几个

```c
// 其次:
/* USER CODE BEGIN 1 */

/*** 定义USART索引枚举 ***/

// !!!!!!!!!!!记得开!!!!!!!!!!!!!!!!
#define USART1_SET
#define USART2_SET
//#define USART3_SET
// !!!!!!!!!!!记得开!!!!!!!!!!!!!!!!

UART_HandleTypeDef* Current_USART_Handle;		  /* 当前某个USART的句柄 */
Current_USART_Indx Current_USART_Printf_Indx;	/* 当前某个USART的索引 */

/* 
 * 简介：设置当前使用的USART
 * 参数：indx - 要设置的USART索引
 * 这个参数可以是：USARTx_IDX，其中x可以从1~3
 * 使用举例：（必须要将其放在printf函数前面，指定其中一个串口）
 * 		Set_Current_USART(USART1_IDX);
 * 		printf("我是串口1\r\n");
 */
void Set_Current_USART(Current_USART_Indx indx)
{
	switch(indx)
  {
		#ifdef USART1_SET
    case USART1_IDX:
    Current_USART_Handle = &huart1;
    Current_USART_Printf_Indx = USART1_IDX;
    break;
		#endif
		#ifdef USART2_SET
    case USART2_IDX:
    Current_USART_Handle = &huart2;
    Current_USART_Printf_Indx = USART2_IDX;
    break;
		#endif
		#ifdef USART3_SET
    case USART3_IDX:
    Current_USART_Handle = &huart3;
    Current_USART_Printf_Indx = USART3_IDX;
    break;
		#endif
    default:
    Current_USART_Handle = NULL;
    Current_USART_Printf_Indx = USART_NONE;
    break;
  }
}

/* 
 * 简介：重定义fputc函数，用于将字符输出到当前设置的USART
 * 参数：
 * ch - 要发送的字符
 * f  - 文件指针（在此实现中未使用）
 * 返回值：发送的字符（或EOF如果出错）
 */

int fputc(int ch, FILE *f)
{
  if(Current_USART_Handle == NULL)
	{			/* 如果当前没有设置USART句柄，则返回EOF表示错误 */
    return EOF;
  }
  /* 根据当前设置的USART句柄，选择对应的USART外设发送字符 */
	#ifdef USART1_SET
  if(Current_USART_Handle == &huart1)
	{		
		while ((USART1->SR & 0X40) == 0); 		/* 等待USART1发送完成，然后发送字符 */
		USART1->DR = (uint8_t)ch; 				/* 将要发送的字符 ch 写入到DR寄存器 */
  }
	#endif
	#ifdef USART2_SET
  else if(Current_USART_Handle == &huart2)
	{
		while ((USART2->SR & 0X40) == 0); 		/* 等待USART2发送完成，然后发送字符 */
		USART2->DR = (uint8_t)ch; 				/* 将要发送的字符 ch 写入到DR寄存器 */
  }
	#endif
	#ifdef USART3_SET
  else if(Current_USART_Handle == &huart3)
	{
		while ((USART3->ISR & 0X40) == 0); 		/* 等待USART3发送完成，然后发送字符 */
		USART3->TDR = (uint8_t)ch; 				/* 将要发送的字符 ch 写入到DR寄存器 */
  }
	#endif
  return ch;								/* 返回发送的字符 */
}


/* USER CODE END 1 */
```

* **④** 在==usart.h==

```c
/* USER CODE BEGIN Prototypes */

/*** 定义USART索引枚举 ***/
typedef enum {
	USART_NONE,			/* 无USART */
	USART1_IDX,			/* USART1索引 */
	USART2_IDX,			/* USART2索引 */
	USART3_IDX,			/* USART3索引 */
}Current_USART_Indx;

extern UART_HandleTypeDef* Current_USART_Handle;		  /* 当前某个USART的句柄 */
extern Current_USART_Indx Current_USART_Printf_Indx;	/* 当前某个USART的索引 */
 
void Set_Current_USART(Current_USART_Indx indx);			/* 函数声明，用于设置当前使用的USART */

/* USER CODE END Prototypes */
```

* 示例代码:

```c
Set_Current_USART(USART1_IDX); /* 想要指定不同串口必须在printf前加上此函数 */
printf("Serial=1\n");
```

