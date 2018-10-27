/*
 * ipd_servo.h
 *
 *  Created on: 14.07.2018
 *      Author: johae
 */

#ifndef INC_IPD_SERVO_H_
#define INC_IPD_SERVO_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include "stdint.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "tim.h"
#include "GUI.h" //STEmWin
#include "WM.h" //STEmWin


#define SERVO_PAN			(1)
#define SERVO_PAN_CH		(&htim12)
#define SERVO_TILT			(2)
#define SERVO_TILT_CH		(&htim3)
#define SERVO_LOW_LIMIT 	(105)		// lower limit for pulse width
#define SERVO_HIGH_LIMIT 	(200)		// upper limit for pulse width
#define SERVO_READY			(1)
#define SERVO_DISABLED		(0)

void servo_init(uint8_t servo);
void servo_deinit(uint8_t servo);
void servo_writeMS (uint8_t servo, uint8_t value);
void servo_writeDEG (uint8_t servo, uint8_t value);

extern osMessageQId servoCmdQueueHandle;

#endif /* INC_IPD_SERVO_H_ */
