/*
 * buttonTask.c
 *
 *  Created on: 2017年7月28日
 *      Author: silezhi
 */
#include "Button_Task.h"

#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_system.h"

const uint8_t c_u8KeyGpio[8] ={GPIO_BUTTON1,GPIO_BUTTON2,GPIO_BUTTON3,GPIO_BUTTON4,GPIO_SWITCH,GPIO_USBSTATE,0xff,0xff};
static uint8_t s_u8KeyState[8] = {0};

static const char *TAG = "Bton_Task";

//回调链表表头指针
static bton_cb_list *bton_cb_list_head = NULL;
static void RunBtonCB(bton_cb_list *bton_cb_list_head, btonParm parm, uint32_t event)
{
	bton_cb_list *list=bton_cb_list_head;
	do{
		list->bton_cb_p(parm,event);
		list=list->next;
	}while(list!=NULL);
}

static void KeyFunction(uint32_t keyScanState,void *pvParameters)
{
	//一个标志 表示是否上一个状态为长按 保证长按之后的短按不会被反应
	static uint8_t L_PUSH_FLAG=0;
	uint32_t event=0;
	btonParm TbtonParm={0};
	uint8_t state=0;
	//按键短按
	//按键A
	if ((keyScanState & BUTTON_RLS(0)) && L_PUSH_FLAG!=1)
	{
		event |= EVENT_RLS;
		TbtonParm.buttonID|=BUTTON_A;
		state=1;
	}
	//按键B
	else if ((keyScanState & BUTTON_RLS(1))  && L_PUSH_FLAG!=2)
	{
		event |= EVENT_RLS;
		TbtonParm.buttonID|=BUTTON_B;
		state=1;
	}
	//按键C
	else if ((keyScanState & BUTTON_RLS(2))  && L_PUSH_FLAG!=3)
	{
		event |= EVENT_RLS;
		TbtonParm.buttonID|=BUTTON_C;
		state=1;
	}
	//按键D
	else if ((keyScanState & BUTTON_RLS(3))  && L_PUSH_FLAG!=4)
	{
		event |= EVENT_RLS;
		TbtonParm.buttonID|=BUTTON_D;
		state=1;
	}
	//震动开关
	else if ((keyScanState & BUTTON_RLS(4))  && L_PUSH_FLAG!=5)
	{
		event |= EVENT_RLS;
		TbtonParm.buttonID|=BUTTON_S;
		state=1;
	}
    //USB检测
    else if ((keyScanState & BUTTON_RLS(5))  && L_PUSH_FLAG!=6)
    {
        event |= EVENT_USBOUT;
        TbtonParm.buttonID|=USB_STA;
        state=1;
    }

    //USB检测
    if (keyScanState & BUTTON_PUSH(5))
    {
        event |= EVENT_USBIN;
        TbtonParm.buttonID|=USB_STA;
        state=1;
    }

	L_PUSH_FLAG = 0;
	//按键长按
	//按键A
	if (keyScanState & BUTTON_PUSH_LT(0))
	{
		ESP_LOGI(TAG,"BUTTON1 LONG PUSH");
		L_PUSH_FLAG=1;
		event |= EVENT_LONGPUSH;
		TbtonParm.buttonID|=BUTTON_A;
		state=1;
	}
	//按键B
	else if (keyScanState & BUTTON_PUSH_LT(1))
	{
		ESP_LOGI(TAG,"BUTTON2 LONG PUSH");
		L_PUSH_FLAG=2;
		event |= EVENT_LONGPUSH;
		TbtonParm.buttonID|=BUTTON_B;
		state=1;
	}
	//按键C
	else if (keyScanState & BUTTON_PUSH_LT(2))
	{
		ESP_LOGI(TAG,"BUTTON3 LONG PUSH");
        L_PUSH_FLAG=3;
		event |= EVENT_LONGPUSH;
		TbtonParm.buttonID|=BUTTON_C;
		state=1;
	}
	//按键D
	else if (keyScanState & BUTTON_PUSH_LT(3))
	{
		ESP_LOGI(TAG,"BUTTON4 LONG PUSH");
        L_PUSH_FLAG=4;
		event |= EVENT_LONGPUSH;
		TbtonParm.buttonID|=BUTTON_D;
		state=1;
	}

	if(state==1)
	{
		RunBtonCB(bton_cb_list_head, TbtonParm, event);
	}
}

static void ButtonGPIOConfiguration(void)
{
    gpio_pad_select_gpio(GPIO_BUTTON1);
    gpio_set_direction(GPIO_BUTTON1, GPIO_MODE_INPUT);

    gpio_pad_select_gpio(GPIO_BUTTON2);
    gpio_set_direction(GPIO_BUTTON2, GPIO_MODE_INPUT);

    gpio_pad_select_gpio(GPIO_BUTTON3);
    gpio_set_direction(GPIO_BUTTON3, GPIO_MODE_INPUT);

    gpio_pad_select_gpio(GPIO_BUTTON4);
    gpio_set_direction(GPIO_BUTTON4, GPIO_MODE_INPUT);

    gpio_pad_select_gpio(GPIO_SWITCH);
    gpio_set_direction(GPIO_SWITCH, GPIO_MODE_INPUT);

    gpio_pad_select_gpio(GPIO_USBSTATE);
    gpio_set_direction(GPIO_USBSTATE, GPIO_MODE_INPUT);
}

static void KeyRead(uint8_t* u8pKeyState)
{
  uint8_t i;
  for (i=0;i<8;i++)
  {
	  if(c_u8KeyGpio[i] != 0xff)
	  {
		  u8pKeyState[i] = !gpio_get_level(c_u8KeyGpio[i]);
	  }
	  else
		  u8pKeyState[i] = 1;
  }
}

// 0- 7bit 代表按键状态变化
// 8-15bit 代表按键长按
//16-23bit 代表按键按下
//24-31bit 代表按键弹起
static uint32_t KeyScan(uint32_t timeTickNow)
{
  static uint8_t s_u8ButtonStateBefore =0xFF;
  static uint32_t s_u32aTimeTickBefore[8] = {0};

  static uint16_t buttonPushFlag=0;
  static uint8_t longPushFlag=0;

  uint8_t u8TButtonStateNow=0;
  uint8_t u8TButtonUp=0;
  uint8_t u8TButtonDown=0;
  uint32_t u32TButtonStateChange=0;

  KeyRead(s_u8KeyState);
  for (int i=0;i<8;i++)
  {
    if (s_u8KeyState[i])u8TButtonStateNow |= (1<<i);
  }

  u32TButtonStateChange = (uint32_t)(u8TButtonStateNow^s_u8ButtonStateBefore);
  s_u8ButtonStateBefore = u8TButtonStateNow;

  for (int i=0;i<8;i++)
  {
    if ( u32TButtonStateChange & (1<<i))
    {
      if ( u8TButtonStateNow & (1<<i))
      {
		u8TButtonUp|=1<<i;
        buttonPushFlag &= (~(1<<i));
      }
      else if (!(buttonPushFlag & (1<<i)))
      {
        s_u32aTimeTickBefore[i] = timeTickNow;
        buttonPushFlag |= (1<<i);
        u8TButtonDown|=1<<i;
      }
    }
    else
    {
      if ( u8TButtonStateNow & (1<<i))
      {
        longPushFlag &= ~(1<<i);
      }
      else
      {
        if ((buttonPushFlag & (1<<i)) && (timeTickNow - s_u32aTimeTickBefore[i])> PUSHLTIME)
        {
          if ((longPushFlag&(1<<i))==0)
          {
            u32TButtonStateChange |= (1<<(i+8));
            longPushFlag|=(1<<i);
          }
        }
        else
        {

        }
      }
    }
	}
	return (uint32_t)u32TButtonStateChange|((uint32_t)u8TButtonUp<<24)|((uint32_t)u8TButtonDown<<16);
}

//时间复杂度O(n)
static bton_cb_list* find_last_node(bton_cb_list *list_head)
{
	bton_cb_list *list=list_head;
	for(;list->next!=NULL;)
	{
		list=list->next;
	}
	return list;
}

int32_t btonCallBackRegister(void* parm,bton_cb_t bton_cb_p)
{
	bton_cb_list *cbTPbtonList = NULL;
	cbTPbtonList = malloc(sizeof(bton_cb_list));
	if(cbTPbtonList==NULL)
		return -1;
	//初始化node
	cbTPbtonList->next=NULL;
	//将回调函数加入链表
	cbTPbtonList->bton_cb_p = bton_cb_p;
	//若第一次注册，将链表头确定
	if(bton_cb_list_head == NULL)
	{
		bton_cb_list_head = cbTPbtonList;
	}
	else
	{
		bton_cb_list* cbT = find_last_node(bton_cb_list_head);
		cbT->next = cbTPbtonList;
	}
	return 1;
}

static void testBtonCB1(btonParm parm,uint32_t event)
{
	if(event & EVENT_RLS)
	{
		if(parm.buttonID & BUTTON_A)
		{
			ESP_LOGI(TAG,"CB1 BUTTON A RLS");
		}
	}

    if(event & EVENT_USBIN)
    {
        ESP_LOGI(TAG,"CB1 USBIN");
    }
    else if (event & EVENT_USBOUT)
    {
        ESP_LOGI(TAG,"CB1 USBOUT");
    }
}

static uint32_t btonGetTimeNow(void)
{
    struct timeval tv = { 0 };
    uint32_t time_ms;

    gettimeofday(&tv, NULL);

    time_ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    return time_ms;
}

void ButtonTask(void *pvParameters)
{
	uint32_t keyScanState=0;
	ESP_LOGI(TAG,"Button Task Start");
	ButtonGPIOConfiguration();
	btonCallBackRegister(NULL,testBtonCB1);
	while(1)
	{
		keyScanState=KeyScan(btonGetTimeNow());
		if (keyScanState!=0)
		{
			KeyFunction(keyScanState,pvParameters);
		}
		vTaskDelay(pdMS_TO_TICKS(50));
	}
	//任务结束 删除
	vTaskDelete(NULL);
}

