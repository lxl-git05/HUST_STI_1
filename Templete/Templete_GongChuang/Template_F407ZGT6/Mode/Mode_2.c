#include "AllHeader.h"

int cmd = 0 ;	// 使用debug模式进行参数修改
float Motor_Cur_Pos = 0.0f;
int16_t Motor_Vel = 0;

// cmd功能表：
// 1速度运行  2停机  3位置模式  4使能控制  5切换开环模式
// 6触发回零  7多机同步  8多电机命令  9快速位置模式
// 10保存单圈零点  11读取实时位置  12读取实时转速

void Mode_2_Setup(void)
{
	
}

void Mode_2_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "===Mode_2===") ;
	// 指令实现
	if (Key_Check(KEY_1 , KEY_SINGLE) || cmd == 1)
	{
		Emm_V5_Vel_Control(1, 0, 500, 10, 0);	// 速度模式: 
	}
	if (Key_Check(KEY_2 , KEY_SINGLE) || cmd == 2)
	{
		Emm_V5_Vel_Control(1, 0, 0, 10, 0);		// 停机!
	}
	if (cmd == 3)
	{
		// 位置模式：CW，100RPM，相对运动3200脉冲
		Emm_V5_Pos_Control(1, 0, 100, 0, 3200, 0, 0);
	}
	if (cmd == 4)
	{
		// 使能控制示例：运动一圈后去使能，再重新使能
		Emm_V5_Pos_Control(1, 0, 1000, 0, 3200, 0, 0);
		HAL_Delay(2000);
		Emm_V5_En_Control(1, 0, 0);
		HAL_Delay(100);
		Emm_V5_En_Control(1, 1, 0);
	}
	if (cmd == 6)
	{
		// 设置临时零点，偏移800脉冲后触发单圈就近回零
		Emm_V5_Origin_Set_O(1, 0);
		HAL_Delay(10);
		Emm_V5_Pos_Control(1, 0, 1000, 0, 800, 0, 0);
		HAL_Delay(1000);
		Emm_V5_Origin_Trigger_Return(1, 0, 0);
	}
	if (cmd == 7)
	{
		// 地址1~4分别加载位置指令，再由广播地址0同步触发
		// 这个同步性很高
		Emm_V5_Pos_Control(1, 0, 1000, 0, 3200, 0, 1);
		HAL_Delay(10);
		Emm_V5_Pos_Control(2, 0, 1000, 0, 3200, 0, 1);
		HAL_Delay(10);
		Emm_V5_Pos_Control(3, 0, 1000, 0, 3200, 0, 1);
		HAL_Delay(10);
		Emm_V5_Pos_Control(4, 0, 1000, 0, 3200, 0, 1);
		HAL_Delay(10);
		Emm_V5_Synchronous_motion(0);
	}
	if (cmd == 8)
	{
		// 将四台电机的运动参数打包成一条多电机命令发送
		Emm_V5_MMCL_Pos_Control(1, 0, 1000, 0, 3200, 0, 0);
		Emm_V5_MMCL_Pos_Control(2, 1, 400, 0, 3200, 0, 0);
		Emm_V5_MMCL_Pos_Control(3, 0, 2000, 0, 32000, 0, 0);
		Emm_V5_MMCL_Pos_Control(4, 0, 1000, 0, 64000, 0, 0);
		Emm_V5_Multi_Motor_Cmd(0);
	}
	if (cmd == 9)
	{
		// 快速位置模式：先到+3200，再到-3200
		Emm_V5_Set_QPos_Params(1, 1000, 0, 1, 0);
		HAL_Delay(10);
		Emm_V5_QPos_Control(1, 3200);
		HAL_Delay(2000);
		Emm_V5_QPos_Control(1, -3200);
	}
	if (cmd == 10)
	{
		// 将当前位置保存为单圈回零的零点
		Emm_V5_Origin_Set_O(1, 1);
	}
	if (cmd == 11)
	{
		// 读取电机实时位置
		Emm_V5_Read_Sys_Params(1, S_CPOS);
	}
	if (cmd == 12)
	{
		// 读取电机实时转速
		Emm_V5_Read_Sys_Params(1, S_VEL);
	}
	
	
	// 指令只允许发送一次
	if (cmd != 0)
		cmd = 0 ;

	// 解析商家例程中的实时位置、实时转速返回帧
	if (rxFrameFlag == true)
	{
		rxFrameFlag = false;
		if (rxCmd[0] == 1 && rxCmd[1] == 0x36 && rxCount == 8)
		{
			uint32_t pos = ((uint32_t)rxCmd[3] << 24) |
							   ((uint32_t)rxCmd[4] << 16) |
							   ((uint32_t)rxCmd[5] << 8)  |
							   ((uint32_t)rxCmd[6]);
			Motor_Cur_Pos = (float)pos * 360.0f / 65536.0f;
			if (rxCmd[2])
				Motor_Cur_Pos = -Motor_Cur_Pos;
		}
		else if (rxCmd[0] == 1 && rxCmd[1] == 0x35 && rxCount == 6)
		{
			uint16_t vel = ((uint16_t)rxCmd[3] << 8) |
							   ((uint16_t)rxCmd[4]);
			Motor_Vel = (int16_t)vel;
			if (rxCmd[2])
				Motor_Vel = -Motor_Vel;
		}
	}
	
	// OLED展示相关参数
	OLED_Printf(0,15,OLED_6X8,"T:%d cmd:%02X %02X %02X %02X",rx_times,rxCmd[0],rxCmd[1],rxCmd[2],rxCmd[3]) ;
	OLED_Printf(0,30,OLED_6X8,"P:%.1f V:%d",Motor_Cur_Pos,Motor_Vel) ;
}

void Mode_2_Tick(void)
{
	
}

void Mode_2_Exit(void)
{
	
}
