/*
 * ipd_visu.h
 *
 *  Created on: 15.07.2018
 *      Author: johae
 */

#ifndef INC_IPD_VISU_H_
#define INC_IPD_VISU_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "tim.h"
#include "GUI.h" //STEmWin
#include "WM.h" //STEmWin
#include "DIALOG.h"

typedef enum { STATE_INITIAL, STATE_WAITING, STATE_SERVO_INIT, STATE_COLOR, STATE_FACE, STATE_CODE, NUM_STATES } state_t;
typedef enum { SERVOSTATE_INITIAL, SERVOSTATE_READY, SERVOSTATE_DISABLED, NUM_SERVOSTATES } servostate_t;
typedef enum { MSG_NONE = 0, MSG_START, MSG_STOP, MSG_DISABLE, MSG_COLOR, MSG_FACE, MSG_CODE, MSG_SERVO, NUM_MSG} msg_t;

extern state_t currState;
extern uint8_t currServoState;
extern osMessageQId funcCmdQueueHandle;
extern WM_HWIN hWin;




#endif /* INC_IPD_VISU_H_ */
