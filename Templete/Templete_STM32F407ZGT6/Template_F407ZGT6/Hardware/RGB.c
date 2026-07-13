#include "RGB.h"
// =========== 初始化 ===========
void RGB_Init(void)
{
    // 默认灭灯(共阳极: 写1=灭)
    MyGPIO_WritePin(&MyGPIO_RGB_R, 0);
    MyGPIO_WritePin(&MyGPIO_RGB_G, 0);
    MyGPIO_WritePin(&MyGPIO_RGB_B, 0);
}

// =========== 设置颜色 ===========
void RGB_Set_Color(int R, int G, int B)
{
    MyGPIO_WritePin(&MyGPIO_RGB_R, R);
    MyGPIO_WritePin(&MyGPIO_RGB_G, G);
    MyGPIO_WritePin(&MyGPIO_RGB_B, B);
}
