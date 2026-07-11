#include "Mode_2.h"
#include "AllHeader.h"
#include "Stepper.h"
#include "Emm_V5.h"

static uint8_t motor_select = 1;  // 0=电机1, 1=电机2

void Mode_2_Setup(void)
{
    OLED_Clear();
    Stepper_Init();  // 步进电机初始化
		Stepper_Angle_Dot_Update() ;	// 参数更新
}

// ============== Mode_2 主循环 ==============
void Mode_2_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "us:%.2f func:%.2f", time_us, time_Func_us);

	if (Key_Check(KEY_1, KEY_SINGLE)) 
	{
		Stepper_Draw_Square(0,0,0.004f,0.5f) ;
	}
	if (Key_Check(KEY_2, KEY_DOUBLE))
	{	
		Stepper_Stop_Enqueue(&Stepper1);  // 停车
		Stepper_Stop_Enqueue(&Stepper2);  // 停车
	}
		
/*
		if (Key_Check(KEY_0, KEY_LONG)) 
		{
        motor_select = !motor_select;
    }

		OLED_Printf(0, 10, OLED_6X8, "Motor:%d", motor_select + 1);
	
    if (motor_select == 0) {
        // ========== 电机1测试 (USART3) ==========
        // KEY_1单击 - 速度模式
        if (Key_Check(KEY_1, KEY_SINGLE)) {
            Stepper_Vel_Enqueue(&Stepper1, 50, 0);  // 速度100RPM
        }
        // KEY_1双击 - 相对运动360度
        if (Key_Check(KEY_1, KEY_DOUBLE)) {
            Stepper_RelPos_Enqueue(&Stepper1, 50, 0, 360.0f);  // 相对运动360度
        }
        // KEY_1长按 - 绝对运动到0度
        if (Key_Check(KEY_1, KEY_LONG)) {
            Stepper_AbsPos_Enqueue(&Stepper1, 50, 0, 0.0f);  // 运动到0度
        }
        // KEY_2长按 - 当前位置设为0点
        if (Key_Check(KEY_2, KEY_LONG)) {
            Stepper_ResetZero_Enqueue(&Stepper1);  // 当前位置设为0点
        }
        // KEY_2单击 - 停车
        if (Key_Check(KEY_2, KEY_SINGLE)) {
            Stepper_Stop_Enqueue(&Stepper1);  // 停车
        }

        // ========== OLED显示 ==========
        OLED_Printf(0, 20, OLED_6X8, "M1 vel:%d", Stepper1.Speed_Now);
        OLED_Printf(0, 30, OLED_6X8, "M1 pos:%.2f", Stepper1.Pos_Now);

        OLED_Printf(0, 40, OLED_6X8, "%x %x %x %x %x %x %x %x",
            Stepper1.rx_buf[0], Stepper1.rx_buf[1], Stepper1.rx_buf[2], Stepper1.rx_buf[3],
            Stepper1.rx_buf[4], Stepper1.rx_buf[5], Stepper1.rx_buf[6], Stepper1.rx_buf[7]);

        // 显示队列状态
        OLED_Printf(0, 50, OLED_6X8, "Q:%d", Stepper1.cmd_queue.count);
    } else {
        // ========== 电机2测试 (USART6) ==========
        // KEY_1单击 - 速度模式
        if (Key_Check(KEY_1, KEY_SINGLE)) {
            Stepper_Vel_Enqueue(&Stepper2, 30, 0);  // 速度100RPM
        }
        // KEY_1双击 - 相对运动180度
        if (Key_Check(KEY_1, KEY_DOUBLE)) {
            Stepper_RelPos_Enqueue(&Stepper2, 20, 0, 30.0f);  // 相对运动30度
        }
        // KEY_1长按 - 绝对运动到0度
        if (Key_Check(KEY_1, KEY_LONG)) {
            Stepper_AbsPos_Enqueue(&Stepper2, 20, 0, 0.0f);  // 运动到0度
        }
        // KEY_2长按 - 当前位置设为0点
        if (Key_Check(KEY_2, KEY_LONG)) {
            Stepper_ResetZero_Enqueue(&Stepper2);  // 当前位置设为0点
        }
        // KEY_2单击 - 停车
        if (Key_Check(KEY_2, KEY_SINGLE)) {
            Stepper_Stop_Enqueue(&Stepper2);  // 停车
        }

        // ========== OLED显示 ==========
        OLED_Printf(0, 20, OLED_6X8, "M2 vel:%d", Stepper2.Speed_Now);
        OLED_Printf(0, 30, OLED_6X8, "M2 pos:%.2f", Stepper2.Pos_Now);

        OLED_Printf(0, 40, OLED_6X8, "%x %x %x %x %x %x %x %x",
            Stepper2.rx_buf[0], Stepper2.rx_buf[1], Stepper2.rx_buf[2], Stepper2.rx_buf[3],
            Stepper2.rx_buf[4], Stepper2.rx_buf[5], Stepper2.rx_buf[6], Stepper2.rx_buf[7]);

        // 显示队列状态
        OLED_Printf(0, 50, OLED_6X8, "Q:%d", Stepper2.cmd_queue.count);
    }
*/
}

// ============== 5ms定时器回调 ==============
// 处理队列命令出队和电机状态查询
void Timer_5ms_Callback(void)
{
    Stepper_Polling_5ms();  // 统一处理两个电机的队列和查询,8us完成
}

void Mode_2_Tick(void)
{
	
}

void Mode_2_Exit(void)
{
}
