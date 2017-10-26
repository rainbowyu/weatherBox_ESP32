/*
 * app_main.h
 *
 *  Created on: 2017年7月27日
 *      Author: silezhi
 */

#ifndef _MAIN_APP_MAIN_H_
#define _MAIN_APP_MAIN_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

#include "stdint.h"

//配置有关的flag
extern EventGroupHandle_t config_event_group;
extern const int CONFIGING_BIT;
extern const int CONFIG_DONE_BIT;
extern const int SMARTCONFIG_DONE_BIT;
//哪些任务正在运行flag
extern EventGroupHandle_t taskRun_event_group;
extern const int SMARTCONFIG_TASK_BIT;
extern const int BUTTON_TASK_BIT;
extern const int MQTT_TASK_BIT;
extern const int GET_DEVICE_TASK_BIT;
//网络状态flag
extern EventGroupHandle_t wifi_event_group;
extern const int CONNECTED_BIT;
//设备状态flag
extern EventGroupHandle_t devstate_event_group;
extern const int NOT_CONFIG_BIT;

//nvs读取互斥锁
extern SemaphoreHandle_t nvsMutexHandle;
//mqtt发送互斥锁
extern SemaphoreHandle_t mqttMutexHandle;

#define CONFIG_POWER_SAVE_MODEM
#ifdef CONFIG_POWER_SAVE_MODEM
#define DEFAULT_PS_MODE WIFI_PS_MODEM
#elif CONFIG_POWER_SAVE_NONE
#define DEFAULT_PS_MODE WIFI_PS_NONE
#else
#define DEFAULT_PS_MODE WIFI_PS_NONE
#endif

extern const char *gc_sMEX_10_FIRM_VERSION;

#endif /* ALIOTT_DEMO4_MAIN_APP_MAIN_H_ */
