/*
 * ipd_visu.c
 *
 *  Created on: 14.07.2018
 *      Author: johae
 */

#include "ipd_visu.h"
#include "ipd_servo.h"

WM_HWIN hWin;
WM_HWIN CreateMainWindow(void);
TS_StateTypeDef  TS_State;
state_t currState;
uint8_t currServoState;

/* StartStatemaschineTask function */
void StartStatemaschineTask(void const * argument)
{
  /* USER CODE BEGIN StartStatemaschineTask */
	currState = STATE_INITIAL;
	currServoState = SERVO_DISABLED;
  /* Infinite loop */
  for(;;)
  {
	  // message from queue
	  msg_t currMsg = MSG_NONE;
	  osEvent msg = osMessageGet(funcCmdQueueHandle,0);
	  if (msg.status == osEventMessage)
	  {
		  currMsg = msg.value.v;
	  }

	  // disable servo tracking
	  if (currMsg == MSG_SERVO && currServoState == SERVO_READY)
	  {
		  servo_deinit(SERVO_PAN);
		  servo_deinit(SERVO_TILT);
		  currServoState = SERVO_DISABLED;
	  }
	  // state machine
	  switch(currState){
	  case STATE_INITIAL:
		  if (currMsg == MSG_START)
		  {
			  currState = STATE_WAITING;
		  }
		  if (currMsg == MSG_SERVO)
		  {
		 	  currState = STATE_SERVO_INIT;
		  }
		  break;
	  case STATE_WAITING:
		  if (currMsg == MSG_SERVO)
		  {
			  currState = STATE_SERVO_INIT;
		  }
		  break;
	  case STATE_COLOR:
		  break;
	  case STATE_SERVO_INIT:
		  if (currServoState != SERVO_READY)
		  {
			 servo_writeDEG(SERVO_PAN,90);
			 servo_writeDEG(SERVO_TILT,0);
			 currServoState = SERVO_READY;
		  }
		  currState = STATE_WAITING;
		  break;
	  case STATE_FACE:
		  break;
	  case STATE_CODE:
		  break;
	  default:
		  Error_Handler();
		  break;

	  }
    osDelay(1);
  }
  /* USER CODE END StartStatemaschineTask */
}



void StartSTemWinTask(void const * argument)
{
  /* USER CODE BEGIN StartSTemWinTask */
	BSP_LCD_Init();
	HAL_Delay(200);
	GUI_Init();
	GUI_SetBkColor(GUI_WHITE);
	GUI_Clear();

	hWin = CreateMainWindow();

  /* Infinite loop */
  for(;;)
  {
    GUI_Delay(500);
    osDelay(1);
  }
  /* USER CODE END StartSTemWinTask */
}


/* StartTouchTask function */
void StartTouchTask(void const * argument)
{
	GUI_PID_STATE STemWin_TS;
	/* USER CODE BEGIN StartTouchTask */
	if (BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize()) != TS_OK) {
		Error_Handler();
	}
	/* Infinite loop */
	for (;;)
	{
		BSP_TS_GetState(&TS_State);
		if (TS_State.touchEventId[0] == TOUCH_EVENT_PRESS_DOWN)
		{
			GUI_SetFont(&GUI_FontComic24B_ASCII);
			STemWin_TS.Pressed = 1;
			STemWin_TS.Layer = 0;
			STemWin_TS.x = TS_State.touchX[0];
			STemWin_TS.y = TS_State.touchY[0];
			BSP_TS_ResetTouchData(&TS_State);
		}
		else
		{
			STemWin_TS.Pressed = 0;
		}
		GUI_TOUCH_StoreStateEx(&STemWin_TS);
		osDelay(1);
	}
	/* USER CODE END StartTouchTask */
}

/* StartCommTask function */
void StartCommTask(void const * argument)
{
  /* USER CODE BEGIN StartCommTask */

  /* Infinite loop */
  for(;;)
  {
	  // uint8_t echo = 'A';
	  //HAL_UART_Transmit_IT(&huart6,&echo, 1);
	  osDelay(1);
  }
  /* USER CODE END StartCommTask */
}
