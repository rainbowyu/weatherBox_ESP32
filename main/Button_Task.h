/*
 * buttonTask.h
 *
 *  Created on: 2017年7月28日
 *      Author: silezhi
 */

#ifndef _BUTTON_TASK_H_
#define _BUTTON_TASK_H_

#include "stdint.h"

#define GPIO_BUTTON1  (4)
#define GPIO_BUTTON2  (22)
#define GPIO_BUTTON3  (21)
#define GPIO_BUTTON4  (25)
#define GPIO_SWITCH   (23)
#define GPIO_USBSTATE (26)
#define PUSHLTIME 3000

#define BUTTON_RLS(i) (uint32_t)(1<<(i+24))
#define BUTTON_PUSH(i) (uint32_t)(1<<(i+16))
#define BUTTON_PUSH_LT(i) (uint32_t)(1<<(i+8))

//button event id
#define EVENT_PUSH     (0x00000001UL)
#define EVENT_RLS      (0x00000002UL)
#define EVENT_LONGPUSH (0x00000004UL)
#define EVENT_USBIN    (0x00000008UL)
#define EVENT_USBOUT   (0x00000010UL)

//button id
#define BUTTON_A     (0x00000001UL)
#define BUTTON_B     (0x00000002UL)
#define BUTTON_C     (0x00000004UL)
#define BUTTON_D     (0x00000008UL)
#define BUTTON_S     (0x00000010UL)
#define USB_STA      (0x00000020UL)


typedef struct {
	void *parm;
	uint32_t buttonID;
}btonParm;

//按键回调函数
typedef void(*bton_cb_t)(btonParm parm,uint32_t event);

//按键回调函数注册链表
typedef struct BTON_CB_LIST{
	bton_cb_t bton_cb_p;
	struct BTON_CB_LIST *next;
}bton_cb_list;

void ButtonTask(void *pvParameters);
int32_t btonCallBackRegister(void* parm,bton_cb_t bton_cb_p);
#endif /* ALIOTT_DEMO_TEST_MAIN_BUTTONTASK_H_ */
