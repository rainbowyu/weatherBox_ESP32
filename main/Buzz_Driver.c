/*
 * buzz_Driver.c
 *
 *  Created on: 2017年8月2日
 *      Author: silezhi
 */
#include "Buzz_Driver.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_log.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"

static const char* TAG="Buzz_Driver";
void BuzzGpioInit(void)
{
	mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, GPIO_BUZZ_PIN);
}

void SetBuzz(uint8_t enable,int fre)
{
	mcpwm_config_t pwm_config;
	if (enable)
	{
		ESP_LOGD(TAG,"set the buzz enable,fre is %d",fre);
		pwm_config.frequency = fre;    //frequency = freHz
		pwm_config.cmpr_a = 50.0;       //duty cycle of PWMxA = 50.0%
		pwm_config.counter_mode = MCPWM_UP_COUNTER;
		pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
	}
	else
	{
		ESP_LOGD(TAG,"set the buzz Disenable");
		pwm_config.frequency = fre;    //frequency = freHZ
		pwm_config.cmpr_a = 0;       //duty cycle of PWMxA = 50.0%
		pwm_config.counter_mode = MCPWM_UP_COUNTER;
		pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
	}
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config);   //Configure PWM1A with above settings
}

void BUZZ_DRIVER_TEST_TASK(void *parm)
{
    BuzzGpioInit();
    ESP_LOGI(TAG,"Buzz Driver Test Start");
    SetBuzz(1,1000);
    vTaskDelay(pdMS_TO_TICKS(3000));
    SetBuzz(0,1000);
    ESP_LOGI(TAG,"Buzz Driver Test End");
    vTaskDelete(NULL);
}
