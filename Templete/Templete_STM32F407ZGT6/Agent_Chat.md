# 1. 模板工程创建

现在我需要你帮我进行工程框架移植，

你现在去到D:\github\HUST_STI\HUST_STI_1\Templete\Template_STM32H743VIT6，读取其内部库文件，比如Mode文件，Hardware文件等，然后构建移植计划，最好是从框架移植开始到功能移植，你现在给我做计划，然后分布移植，每次移植我都要进行检查，注意，移植的时候不要随增减注释，开始计划吧

+++

首先移植Mode文件和Mymain以及AllHeader、MySystem，这是整体架构，我要从架构层面一步一步来

+++

现在我的工程加入了串口DMA(USART0和1)，我想要更新当前的Serial，采用串口DMA进行发送(Serial_Printf)和接收(DMA空闲中断)，先不理USART3和6，然后逻辑保持不变，你来进行计划，列出需要新增、修改、删除什么，我同意之后实施















