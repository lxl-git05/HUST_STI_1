#include "Control.h"
#include "Queue.h"
#include "AllHeader.h"

Queue_Typedef Hanger_Queue ;

Hanger_Status_Typedef Hanger_Curr = Robot_Init ;
Hanger_Status_Typedef Hanger_Next = Robot_Init ;

extern int Robot_Wait_Cmd ; 
int Hanger_Last_Cnt = 0 ;

void Robot_Nothing(void)
{
	;
}

// ====================== 1. Robot_Init ======================
// 1.1 Setup
void Robot_Init_Setup(void)
{
	Hanger_Last_Cnt = HAL_GetTick() ;
}
// 1.2 Run
void Robot_Init_Run(void)
{
	// 丝杆在顶端, 夹子张开,晾衣杆在另一侧(Back),衣架闭合
	// 1. 丝杆在顶端
	Motor_Hang_Up() ;
	// 2. 夹子张开
	Servo_Claw_Open() ;
	// 3. 两个衣架都是闭合
	Servo_Hanger_Close() ;
	Servo_Hanger2_Close() ;
}
// 1.3 Exit
bool Robot_Init_Is_Exit(int WaitTime_ms)
{
	return HAL_GetTick() > Hanger_Last_Cnt + WaitTime_ms ;
}

// ====================== 2. Robot_Down ======================
// 2.1 Setup
// 2.2 Run
void Robot_Down_Run(void)
{
	// 1. 电机向下够衣服
	Motor_Hang_Down() ;
}
// 2.3 Exit
bool Robot_Down_Is_Exit(void)
{
	return Is_Motor_Hanger_Down() ;
}

// ====================== 3. Robot_Claw_Close ======================
// 3.1 Setup
void Robot_Claw_Close_Setup(void)
{
	Hanger_Last_Cnt = HAL_GetTick() ;
}
// 3.2 Run
void Robot_Claw_Close_Run(void)
{
	// 1. 夹衣服
	Servo_Claw_Close() ;
}
// 3.3 Exit
bool Robot_Claw_Close_Is_Exit(int WaitTime_Ms)
{
	return HAL_GetTick() > Hanger_Last_Cnt + WaitTime_Ms ;
}

// ====================== 4. Robot_Mid ======================
// 4.1 Setup
void Robot_Mid_Setup(void)
{
    Hanger_Last_Cnt = HAL_GetTick();
}

// 4.2 Run
void Robot_Mid_Run(void)
{
    // 电机上升,使得衣架能够穿过衣服
    Motor_Hang_Mid();
}

// 4.3 Exit
bool Robot_Mid_Is_Exit(void)
{
    return Is_Motor_Hang_Mid();
}

// ====================== 5. Robot_Hanger1_Open ======================
// 5.1 Setup
void Robot_Hanger1_Open_Setup(void)
{
    Hanger_Last_Cnt = HAL_GetTick();
}

// 5.2 Run
void Robot_Hanger1_Open_Run(void)
{
    // 衣架张开
    Servo_Hanger_Open();
}

// 5.3 Exit
bool Robot_Hanger1_Open_Is_Exit(int WaitTime_ms)
{
    return HAL_GetTick() > Hanger_Last_Cnt + WaitTime_ms;
}

// ====================== 6. Robot_Claw_Open ======================
// 6.1 Setup
void Robot_Claw_Open_Setup(void)
{
    Hanger_Last_Cnt = HAL_GetTick();
}

// 6.2 Run
void Robot_Claw_Open_Run(void)
{
    // 夹爪松开,放开衣服
    Servo_Claw_Open();
}

// 6.3 Exit
bool Robot_Claw_Open_Is_Exit(int WaitTime_ms)
{
    return HAL_GetTick() > Hanger_Last_Cnt + WaitTime_ms;
}

// ====================== 7. Robot_Up ======================
// 7.1 Setup
void Robot_Up_Setup(void)
{
    Hanger_Last_Cnt = HAL_GetTick();
}

// 7.2 Run
void Robot_Up_Run(void)
{
    // 电机上升回位
    Motor_Hang_Up();
}

// 7.3 Exit
bool Robot_Up_Is_Exit(void)
{
    return IS_Motor_Hang_Up();
}

// ====================== 8. Robot_SiGan_Next ======================
int Hua_Pos = 0 ;	// 丝杆滑动
// 8.1 Setup
void Robot_SiGan_Next_Setup(void)
{
    Hanger_Last_Cnt = HAL_GetTick();
		Hua_Pos += Motor_Sigan_Next_Cnt ;
}

// 8.2 Run
void Robot_SiGan_Next_Run(void)
{
    // 丝杆下一阶段
		Motor_SetAngle(&Motor_A , Hua_Pos) ;
}

// 8.3 Exit
bool Robot_SiGan_Next_Is_Exit(void)
{
    return Motor_Is_Angle(&Motor_A, Hua_Pos, 20) ;
}

// ====================== 9. Robot_OK_1 ======================
void Robot_OK_1_Setup(void)
{
    Robot_Wait_Cmd = Cmd_Robot_1_OK ;
}

void Robot_OK_2_Setup(void)
{
	Robot_Wait_Cmd = Cmd_Robot_2_OK ;
}


// ====================== 10. Robot_Hanger2_Open ======================
// 10.1 Setup
void Robot_Hanger2_Open_Setup(void)
{
    Hanger_Last_Cnt = HAL_GetTick();
}

// 10.2 Run
void Robot_Hanger2_Open_Run(void)
{
    // 衣架张开
    Servo_Hanger2_Open();
}

// 10.3 Exit
bool Robot_Hanger2_Open_Is_Exit(int WaitTime_ms)
{
    return HAL_GetTick() > Hanger_Last_Cnt + WaitTime_ms;
}


// ====================== 控制台 ======================

// 队列填充
void Control_Setup(void)
{
	// 初始化
	Queue_Init(&Hanger_Queue) ;
	// 入队
	Queue_Enqueue(&Hanger_Queue , Robot_Init) ;					// 初始化,丝杆在顶端, 夹子张开,传送带位置与衣架对应,衣架闭合
	Queue_Enqueue(&Hanger_Queue , Robot_Down) ;					// 电机向下够衣服
	Queue_Enqueue(&Hanger_Queue , Robot_Claw_Close) ;		// 夹衣服
	Queue_Enqueue(&Hanger_Queue , Robot_Mid) ;					// 电机上升到中位线
	Queue_Enqueue(&Hanger_Queue , Robot_Hanger1_Open) ;	// 衣架张开
	Queue_Enqueue(&Hanger_Queue , Robot_Claw_Open) ;		// 夹爪松开(张开)
	Queue_Enqueue(&Hanger_Queue , Robot_Up) ;						// 电机重新上升到顶点
	Queue_Enqueue(&Hanger_Queue , Robot_SiGan_Next) ;		// 传送带移动，整个过程完成1轮
	Queue_Enqueue(&Hanger_Queue , Robot_OK_1) ;					// 第1轮完成
	// 第2轮
	Queue_Enqueue(&Hanger_Queue , Robot_Down) ;					// 电机向下够衣服
	Queue_Enqueue(&Hanger_Queue , Robot_Claw_Close) ;		// 夹衣服
	Queue_Enqueue(&Hanger_Queue , Robot_Mid) ;					// 电机上升到中位线
	Queue_Enqueue(&Hanger_Queue , Robot_Hanger2_Open) ;	// 衣架2张开
	Queue_Enqueue(&Hanger_Queue , Robot_Claw_Open) ;		// 夹爪松开(张开)
	Queue_Enqueue(&Hanger_Queue , Robot_Up) ;						// 电机重新上升到顶点
	Queue_Enqueue(&Hanger_Queue , Robot_SiGan_Next) ;		// 传送带移动，整个过程再次完成1轮
	Queue_Enqueue(&Hanger_Queue , Robot_OK_2) ;					// 第2轮完成
}

// 出队操作
void Queue_Status_Next(void)
{
    QueueData_Typedef Next_Status ;
    if(Queue_Dequeue(&Hanger_Queue,&Next_Status))
    {
        Hanger_Next = Next_Status;
    }
}

// 控制台
void Hanger_Control(void)
{
    if (Hanger_Curr == Hanger_Next)
    {
        switch (Hanger_Curr)
        {
					case Robot_Init: 					Robot_Init_Run(); 							break;
					case Robot_Down: 					Robot_Down_Run(); 							break;
					case Robot_Claw_Close: 		Robot_Claw_Close_Run(); 				break;
					case Robot_Mid: 					Robot_Mid_Run();							  break;
					case Robot_Hanger1_Open: 	Robot_Hanger1_Open_Run(); 			break;
					case Robot_Claw_Open: 		Robot_Claw_Open_Run(); 					break;
					case Robot_Up: 						Robot_Up_Run(); 								break;
					case Robot_SiGan_Next: 		Robot_SiGan_Next_Run(); 				break;
					case Robot_OK_1: 					Robot_Nothing(); 								break;
					
					case Robot_Hanger2_Open: 	Robot_Hanger2_Open_Run(); 			break;
					case Robot_OK_2: 					Robot_Nothing(); 								break;
        }
    }

    // 状态发生变化时执行Setup
    if (Hanger_Curr != Hanger_Next)
    {
        switch (Hanger_Next)
        {
            case Robot_Init:  					Robot_Init_Setup(); 				break;
            case Robot_Down:  					Robot_Nothing();    				break;
            case Robot_Claw_Close: 			Robot_Claw_Close_Setup(); 	break;
            case Robot_Mid: 						Robot_Mid_Setup(); 					break;
            case Robot_Hanger1_Open: 		Robot_Hanger1_Open_Setup(); break;
            case Robot_Claw_Open: 			Robot_Claw_Open_Setup(); 		break;
            case Robot_Up: 							Robot_Up_Setup(); 					break;
            case Robot_SiGan_Next: 			Robot_SiGan_Next_Setup(); 	break;
						case Robot_OK_1:						Robot_OK_1_Setup();					break;
						case Robot_OK_2:						Robot_OK_2_Setup();					break;
					
						case Robot_Hanger2_Open: 		Robot_Hanger2_Open_Setup(); break;
            default: break;
        }
    }
    // 状态更新
    Hanger_Curr = Hanger_Next;
}

void Hanger_Control_Change(int *Control_Cmd)
{
    if (Hanger_Curr != Hanger_Next)
    {
        return;
    }
		
    switch (Hanger_Curr)
    {
        case Robot_Init:
            // 等待外部启动命令
            if (*Control_Cmd == Cmd_Robot_1_Go)
            {
								Queue_Status_Next() ;
            }
            break;

        case Robot_Down:
            if (Robot_Down_Is_Exit())
            {
								Queue_Status_Next() ;
            }
            break;

        case Robot_Claw_Close:
            if (Robot_Claw_Close_Is_Exit(2000))
            {
									Queue_Status_Next() ;
            }
            break;

        case Robot_Mid:
            if (Robot_Mid_Is_Exit())
            {
									Queue_Status_Next() ;
            }
            break;

        case Robot_Hanger1_Open:
            if (Robot_Hanger1_Open_Is_Exit(1500))
            {
									Queue_Status_Next() ;
            }
            break;

        case Robot_Claw_Open:
            if (Robot_Claw_Open_Is_Exit(600))
            {
									Queue_Status_Next() ;
            }
            break;

        case Robot_Up:
            if (Robot_Up_Is_Exit())
            {
									Queue_Status_Next() ;
            }
            break;

        case Robot_SiGan_Next:
            if (Robot_SiGan_Next_Is_Exit())
            {
									Queue_Status_Next() ;
            }
            break;

        case Robot_OK_1:
            // 一轮完成，等待下一轮的命令
						if ( *Control_Cmd == Cmd_Robot_2_Go )
						{
								Queue_Status_Next() ;
						}

            break;
				case Robot_Hanger2_Open:
            if (Robot_Hanger2_Open_Is_Exit(1500))
            {
								Queue_Status_Next() ;
            }
            break;
				case Robot_OK_2:
            // 完成了
						// 状态锁存在这里
            break;
    }
}






