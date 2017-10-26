/*
 * buzz_Driver.h
 *
 *  Created on: 2017年8月2日
 *      Author: silezhi
 */

#ifndef _BUZZ_DRIVER_H_
#define _BUZZ_DRIVER_H_
#include "stdint.h"

#define GPIO_BUZZ_PIN 2
void BuzzGpioInit(void);
void SetBuzz(uint8_t enable,int fre);
void BUZZ_DRIVER_TEST_TASK(void *parm);
#endif /* ALIOTT_DEMO_TEST_MAIN_BUZZ_DRIVER_H_ */
