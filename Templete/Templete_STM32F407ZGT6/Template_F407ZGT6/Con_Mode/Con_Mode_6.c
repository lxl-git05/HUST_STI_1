// ========================== Con_Mode_6 ==========================
// 题6: 小车置于 A 点，钢球置于摆杆任意指定位置，按键启动后沿黑线顺时针行驶一圈
//       并通过A位置，整圈行驶总时间≤30s，行驶过程中钢球稳定在指定位置附近，误差≤1cm。
//
// 从 Mode_4 覆盖移植: S曲线速度规划 + 弯道3档阻尼 + 终点过线停表
// ★ 球目标: 编码器调节 (mm), KEY2归零, Oran_Single_Pos=39px/cm → 转换为 px
#include "Con_Mode_6.h"

// ==================== 状态机 ====================
enum { S_IDLE, S_RUN, S_FINISH, S_STOP };

// ==================== S 曲线速度规划器 ====================
typedef struct { float v0, vt; int dur, tick; } SCurve_t;

static void scurve_start(SCurve_t *sc, float from, float to, int dur)
{ sc->v0=from; sc->vt=to; sc->dur=(dur>0)?dur:0; sc->tick=0; }
static float scurve_peek(SCurve_t *sc)
{
    if(sc->dur<=0||sc->tick>=sc->dur) return sc->vt;
    float t=(float)sc->tick/sc->dur, s=t*t*(3.0f-2.0f*t);
    return sc->v0+(sc->vt-sc->v0)*s;
}
static float scurve_tick(SCurve_t *sc)
{
    if(sc->dur<=0) return sc->vt; sc->tick++;
    if(sc->tick>=sc->dur) return sc->vt;
    float t=(float)sc->tick/sc->dur, s=t*t*(3.0f-2.0f*t);
    return sc->v0+(sc->vt-sc->v0)*s;
}

// ==================== 偏航角分段速度表 ====================
typedef struct { float yaw; float speed; int ramp; } SpeedSeg_t;
#define SEG_COUNT 6
static SpeedSeg_t seg[SEG_COUNT]={
    {   0.0f, 78.0f,150},{  50.0f, 78.0f,  0},
    { 100.0f, 58.0f, 85},{ 150.0f, 58.0f,  0},
    { 210.0f, 78.0f, 85},{ 280.0f, 78.0f,  0},
};
#define FINISH_MIN_YAW 320.0f

// ==================== 球偏移 (mm→px) ====================
#define OFF_STEP_MM 1.0f
#define OFF_MAX_MM  110.0f
static float s_off_mm=0.0f;
static float mm2px(float mm){return mm*(float)Oran_Single_Pos/10.0f;}

// ==================== 运行时状态 ====================
static uint8_t  state=S_IDLE; static uint32_t t0=0; static float race_time=0.0f;
static SCurve_t sc; static int seg_idx=0;
static float prev_off=0.0f; static uint8_t finish_hold=0;

static int profile_find_seg(float yaw){
    int idx=seg_idx;
    for(int i=seg_idx+1;i<SEG_COUNT;i++){if(yaw>=seg[i].yaw)idx=i;else break;}
    return idx;
}
static void Con6_IntSep(void){
    float r=fabs(PID_Oran.realPoint_Now),s=fabs((float)Oran_Speed);
    if(r>100.0f&&s>30.0f) PID_Oran.SumError=0.0f;
}

// ==================== Setup ====================
void Con_Mode_6_Setup(void)
{
    Oran_PID_Init(); PID_Param_Reset(&PID_Oran);
    PID_Oran.PID_Func=Con6_IntSep;
    PID_Oran.Kp=0.20f; PID_Oran.goalPoint=0.0f; PID_Oran.ioutMax=2000.0f;
    Oran_Real_Offset=mm2px(s_off_mm); Oran_FF_Enable=1.0f; Oran_Damping_K=0.10f;
    Stepper_PWM_Angle_Gains_Set(&Stepper1,4.0f,0.0f,0.829f,50.0f,-50.0f);
    Y8U_SetSpeed(0); state=S_IDLE; seg_idx=0;
    scurve_start(&sc,0,seg[0].speed,seg[0].ramp);
}

// ==================== Loop ====================
void Con_Mode_6_Loop(void)
{
    // ── 编码器调节偏移 ──
    { int16_t enc=Encoder_Get();
      if(enc!=0){s_off_mm+=(float)enc*OFF_STEP_MM;
                 if(s_off_mm>OFF_MAX_MM)s_off_mm=OFF_MAX_MM;
                 else if(s_off_mm<-OFF_MAX_MM)s_off_mm=-OFF_MAX_MM;
                 Oran_Real_Offset=mm2px(s_off_mm);} }
    if(Key_Check(KEY_2,KEY_SINGLE)){s_off_mm=0.0f; Oran_Real_Offset=0.0f;}

    OLED_Printf(0,0,OLED_6X8,"Con_M6 off:%+.0fmm",s_off_mm);

    if(Serial_GetNewPackageFlag_ABC(&Serial1)){
        Serial_SetFloatData(&Serial1,"Kp","Kp=%f",&Y8U_PID.Kp);
        Serial_SetFloatData(&Serial1,"Ki","Ki=%f",&Y8U_PID.Ki);
        Serial_SetFloatData(&Serial1,"Kd","Kd=%f",&Y8U_PID.Kd);
        Serial_SetFloatData(&Serial1,"Goal","Goal=%f",&Oran_Real_Offset);
        Serial_SetFloatData(&Serial1,"Offset","%f",&s_off_mm);
        Serial_SetFloatData(&Serial1,"S0","%f",&seg[0].speed);
        Serial_SetFloatData(&Serial1,"S1","%f",&seg[1].speed);
        Serial_SetFloatData(&Serial1,"S2","%f",&seg[2].speed);
        Serial_SetFloatData(&Serial1,"S3","%f",&seg[3].speed);
        Serial_SetFloatData(&Serial1,"S4","%f",&seg[4].speed);
        Serial_SetFloatData(&Serial1,"S5","%f",&seg[5].speed);
    }
    switch(state){
    case S_IDLE:
        OLED_Printf(0,10,OLED_6X8,"KEY1:Go");
        OLED_Printf(0,20,OLED_6X8,"Speeds:%.0f/%.0f/%.0f",seg[1].speed,seg[3].speed,seg[5].speed);
        if(Key_Check(KEY_1,KEY_SINGLE))
				{
            state=S_RUN; t0=HAL_GetTick(); seg_idx=0;
            IMU_Yaw_Abs_Reset(); Y8U_FinishLine_Reset();
            scurve_start(&sc,10.0f,seg[0].speed,seg[0].ramp); Y8U_SetSpeed(10.0f);
        }
        break;
    case S_RUN:
        OLED_Printf(0,10,OLED_6X8,"S%d(%.0f) D:%.2f",seg_idx,seg[seg_idx].speed,Oran_Damping_K);
        OLED_Printf(0,20,OLED_8X16,"%.1fs %.0frpm",(HAL_GetTick()-t0)/1000.0f,Y8U_GetSpeed());
        break;
    case S_FINISH:
        OLED_Printf(0,10,OLED_6X8,"Decel... %.0frpm",Y8U_GetSpeed());
        OLED_Printf(0,20,OLED_8X16,"%.1fs",(HAL_GetTick()-t0)/1000.0f);
        break;
    case S_STOP:
        OLED_Printf(0,10,OLED_6X8,"Done!"); OLED_Printf(0,20,OLED_8X16,"%.1fs",race_time);
        break;
    }
}

// ==================== Tick (20ms) ====================
void Con_Mode_6_Tick(void)
{
    Oran_Update(); Oran_PID_Update();
    Motor_Pos_Update(&Motor_A); Motor_Pos_Update(&Motor_B);
		Serial_printf(&Serial1 , "%.1f,%.1f,%.1f,%.1f,%.1f\n",PID_Oran.goalPoint, PID_Oran.realPoint_Now
								,PID_Oran.setPoint, IMU_Yaw_Abs_Get(),Y8U_GetSpeed());
		
    if(state==S_IDLE||state==S_STOP) return;

    float yaw=IMU_Yaw_Abs_Get();
    if(state==S_RUN){
        int ni=profile_find_seg(yaw);
        if(ni!=seg_idx){float c=scurve_peek(&sc); scurve_start(&sc,c,seg[ni].speed,seg[ni].ramp); seg_idx=ni;}
        float spd=scurve_tick(&sc); Y8U_SetSpeed(spd);
        if(finish_hold==0) Y8U_PID_Update(); else finish_hold--;
        {
            float off=fabs(Y8U_GetOffset());
            if(off>120.0f) Oran_Damping_K=0.26f; else if(off>60.0f) Oran_Damping_K=0.18f; else Oran_Damping_K=0.08f;
            if(prev_off>60.0f&&off<=60.0f) PID_Oran.SumError=0.0f; prev_off=off;
        }
        if(yaw>FINISH_MIN_YAW&&Y8U_CheckFinishLine()){
            state=S_FINISH; race_time=(HAL_GetTick()-t0)/1000.0f; finish_hold=5;
            float c=scurve_peek(&sc); scurve_start(&sc,c,0.0f,170);
        }
    }else if(state==S_FINISH){
        float spd=scurve_tick(&sc); Y8U_SetSpeed(spd);
        if(finish_hold==0) Y8U_PID_Update(); else finish_hold--;
        {
            float off=fabs(Y8U_GetOffset());
            if(off>120.0f) Oran_Damping_K=0.26f; else if(off>60.0f) Oran_Damping_K=0.18f; else Oran_Damping_K=0.08f;
        }
        if(spd<1.5f){
            state=S_STOP; Y8U_SetSpeed(0); Motor_SetSpeed(&Motor_A,0); Motor_SetSpeed(&Motor_B,0);
        }
    }
}

// ==================== Exit ====================
void Con_Mode_6_Exit(void)
{ state=S_IDLE; Y8U_SetSpeed(0); Motor_SetSpeed(&Motor_A,0); Motor_SetSpeed(&Motor_B,0); Stepper_PWM_Stop(&Stepper1); }
