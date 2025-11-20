#include "MyTask.h"
// ======================= 两个任务结构体 =======================

// 周期执行结构体
typedef struct 
{
	uint8_t Flag;							// 任务延时完成标志位
	uint32_t cnt; 						// 任务计时计数器
	uint32_t cycle;						// 任务执行周期长度
	uint8_t Enable;						// 任务启动标志位
	void (*callback)(void);   // 新增：回调函数指针
}MyTask_Cycle;

// 单次任务结构体
typedef struct
{
    uint8_t busy;
    uint32_t delay;
    uint32_t timer;
    int id;
    void (*post_func)(void);
}MyTask_Once;

// ======================= 全局变量 =======================
// 周期任务执行表
#define MyTask_MAX_NUM 30										// 周期任务总数,最多执行30个任务,否则报错(报错就加大MAX呗)
MyTask_Cycle MyTask_List[MyTask_MAX_NUM] = {0} ;	
int MyTask_Cycle_Num = 0 ;									// 系统周期任务总数计数
// 单次任务执行表
MyTask_Once OnceTask_List[10] = {0};
int MyTask_Once_Arr[10] = {0} ;							// 单次任务注册表,为0表示没有执行过,为1表示已经执行完毕

// ======================= 函数 =======================

// 周期任务 : 增加任务到任务总表
void MyTask_Cycle_Add_New_Task(uint32_t cnt_init,uint32_t cycle_init , void (*callback_func)(void) , bool is_Task_GO_Now , int *Task_Seq)
{
	// 参数添加
	MyTask_List[MyTask_Cycle_Num].cnt = cnt_init ;					// 计时起点(初相位,与其他任务错开,防止抢占资源)
	MyTask_List[MyTask_Cycle_Num].cycle = cycle_init ;			// 计时周期
	MyTask_List[MyTask_Cycle_Num].Enable = is_Task_GO_Now ;	// 任务是否在建立之后就立马执行
	MyTask_List[MyTask_Cycle_Num].callback = callback_func;	// 任务延迟完成后执行回调函数
	MyTask_List[MyTask_Cycle_Num].Flag = 0 ;								// 任务延迟完成标志位
	// 得到任务码(MyTask_Cycle_Num) , 任务总数+1
	if (Task_Seq != NULL)
		*Task_Seq = MyTask_Cycle_Num ;
	MyTask_Cycle_Num ++ ;
}

// 周期任务 : 开启任务
void MyTask_Cycle_Start_Task(int Task_Seq)
{
	MyTask_List[Task_Seq].Enable = 1 ;
}

// 周期任务 : 暂停任务
void MyTask_Cycle_Stop_Task(int Task_Seq)
{
	// 任务可变参数清零
	MyTask_List[Task_Seq].cnt = 0 ;
	MyTask_List[Task_Seq].Enable = 0 ;
	MyTask_List[Task_Seq].Flag = 0 ;
}

// 单次任务 : 执行一次 前置任务 + 延迟 + 后置任务 后注销 , 即使在while反复调用
// 执行码:必须和其他的单次任务的执行码不同,否则无法再次执行
void MyTask_Once_exe(int id , uint32_t wait_ms , void (*pre_func)(void), void (*post_func)(void))
{
    if (MyTask_Once_Arr[id] == 1)
        return;  // 已经执行过，不再执行

    // 前置任务执行一次
    if (pre_func != NULL)
        pre_func();

    // 找一个单次任务执行表空位插入任务
    for (int i = 0; i < 10; i++)
    {
        if (!OnceTask_List[i].busy)
        {
            OnceTask_List[i].busy = 1;
            OnceTask_List[i].delay = wait_ms;
            OnceTask_List[i].timer = 0;
            OnceTask_List[i].id = id;
            OnceTask_List[i].post_func = post_func;
            return;
        }
    }
}


// 单次任务 : 列表执行任务
void Mytask_Once_Possess(void)
{
	// 扫描单次任务表,检测有无空闲位置进行单次任务执行
	for (int i = 0; i < 10; i++)
	{
		// 检测到有单次任务执行
		if (OnceTask_List[i].busy)
		{
			// 计时器++
			OnceTask_List[i].timer++;
			// 延迟完毕,执行后置回调函数
			if (OnceTask_List[i].timer >= OnceTask_List[i].delay)
			{
				if (OnceTask_List[i].post_func)
						OnceTask_List[i].post_func();
				
				// 注册表置1,从此注销本单次任务
				MyTask_Once_Arr[OnceTask_List[i].id] = 1;
				
				// 单次任务执行表重新回到空闲状态
				OnceTask_List[i].busy = 0;	
				OnceTask_List[i].timer = 0;
				OnceTask_List[i].post_func = NULL;
			}
		}
	}
}


// 周期任务 : 列表执行任务
void MyTask_Cycle_Possess(void)
{
	// 周期任务扫描执行
	for (int i = 0 ; i < MyTask_MAX_NUM ; i++)
	{
		// 如果任务可执行则cnt ++ 
		if (MyTask_List[i].Enable == 1)
		{
			MyTask_List[i].cnt ++ ;
			if (MyTask_List[i].cnt >= MyTask_List[i].cycle)
			{
				MyTask_List[i].Flag = 1 ;
				MyTask_List[i].cnt  = 0 ;
				if (MyTask_List[i].callback != NULL)
					MyTask_List[i].callback() ;
			}
		}
	}
}

// 任务执行总函数
void MyTask_Possess(void)
{
	MyTask_Cycle_Possess() ;
	Mytask_Once_Possess () ;
}


