#ifndef __CON_CAR_H
#define __CON_CAR_H

typedef enum
{
	
	Car_Cross_Init_way ;			// 小车初始点位面对的一段短直线路程
	
	Car_Turn_L_In_way ;				// 小车在岔路口的内圈转弯中
	Car_Turn_R_In_way ;				// 小车在岔路口的外圈转弯中
	
	Car_Cross_L_In_way ; 			// 小车在内圈直线
	Car_Cross_R_In_way ; 			// 小车在外圈直线
	
	Car_Turn_L_Out_way	;			// 小车在内圈转弯岔路出口
	Car_Turn_R_Out_way	;			// 小车在外圈转弯岔路出口
	
	Car_Cross_Oppo_way	;			// 小车在初始点位对面的长直线
	
	Car_Turn_Oppo_way		;			// 小车在初始点斜对面的弯道
	
	Car_Cross_Short_way ;			// 小车在RL线对面的短直道
	
	Car_Turn_Last_way ;				// 小车在最后的弯道
	
	Car_Cross_Last_way ;			// 小车在最后的冲刺直道
	
}Car_Position_Typedef ;

extern Car_Position_Typedef Car_Pos ;


#endif
