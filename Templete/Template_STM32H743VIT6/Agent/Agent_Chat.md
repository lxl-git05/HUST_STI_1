我在D:\github\HUST_STI\HUST_STI_1\Templete\Template_STM32H743VIT6\Agent放入了TI驱动的Encoder工程，作为参考，怎么样更好的进行跨芯片移植，我在想直接Encoder作为一个库，GPIO外部中断再作为一个库，但是Encoder里外部中断算入Encoder库中，那么你需要考虑

1. Encoder两个GPIO算作一个整体，毕竟是编码器
2. 

