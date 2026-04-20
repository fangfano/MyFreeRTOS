/* Private includes -----------------------------------------------------------*/
//includes
#include "user_TasksInit.h"
#include "main.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/


/**
  * @brief  Key press check task
  * @param  argument: Not used
  * @retval None
  */
void KeyTask(void *argument)
{
	uint8_t keystr=0; // 准备发送给 UI 任务的按键编号（1 或 2）
	// 占位符变量，用于向“休眠任务”或“唤醒任务”发送信号。
	// 在 RTOS 中，消息队列有时不需要传递具体数值，只要“有消息”这个动作本身就是信号。
	uint8_t Stopstr=0;
	uint8_t IdleBreakstr=0;
	while(1)
	{
//		// 调用底层驱动函数读取硬件按键状态，按键扫描
//		switch(KeyScan(0))
//		{
//			case 1: // 返回 1 代表按下了 Key 1
//				keystr = 1;
//				// 消息队列
//				// 把“按键 1”的消息丢进 UI 处理队列。这通常会导致屏幕执行“返回”或“向上”的操作。
//				osMessageQueuePut(Key_MessageQueue, &keystr, 0, 1);
//				// 告诉系统“有人动了”。如果此时屏幕是黑的或暗的，系统收到这个消息会立刻恢复亮度。
//				osMessageQueuePut(IdleBreak_MessageQueue, &IdleBreakstr, 0, 1);
//				break;
//
//			case 2:
//				if(Page_Get_NowPage()->page_obj == &ui_HomePage)
//				{
//					// 触发休眠逻辑。这意味着在主页按 Key 2，手表会直接进入关屏休眠状态。
//					osMessageQueuePut(Stop_MessageQueue, &Stopstr, 0, 1);
//				}
//				else
//				{
//					// 逻辑同 Key 1，发送按键编号并触发唤醒信号。这通常用于在菜单里执行“确认”或“翻页”。
//					keystr = 2;
//					osMessageQueuePut(Key_MessageQueue, &keystr, 0, 1);
//					osMessageQueuePut(IdleBreak_MessageQueue, &IdleBreakstr, 0, 1);
//				}
//				break;
//		}
//		// 让出 CPU 使用权 1 毫秒，避免一直处理这个
//		// 如果不加这一行，这个 while(1) 循环会疯狂占用 CPU 资源，导致其他任务（如显示图像）卡顿
	    osDelay(100);
	}
}
