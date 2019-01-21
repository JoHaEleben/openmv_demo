/*
 * ipd_servo.c
 *
 *  Created on: 14.07.2018
 *      Author: johae
 */

#include "ipd_servo.h"
#include "ipd_visu.h"
#include "utils.h"

int32_t improved_map(int32_t value, int32_t minIn, int32_t maxIn, int32_t minOut, int32_t maxOut)
{
    const int32_t rangeIn = maxIn - minIn;
    const int32_t rangeOut = maxOut - minOut;
    const int32_t deltaIn = value - minIn;
    // fixed point math constants to improve accurancy of divide and rounding
    const int32_t fixedHalfDecimal = 1;
    const int32_t fixedDecimal = fixedHalfDecimal * 2;

    return ((deltaIn * rangeOut * fixedDecimal) / (rangeIn) + fixedHalfDecimal) / fixedDecimal + minOut;
}


void servo_init(uint8_t servo)
{
	// TODO: Do something?!
}

void servo_deinit(uint8_t servo)
{
	if (servo == SERVO_PAN)
	{
		HAL_TIM_PWM_Stop(SERVO_PAN_CH, TIM_CHANNEL_1);
	}
	else if (servo == SERVO_TILT)
	{
		HAL_TIM_PWM_Stop(SERVO_TILT_CH, TIM_CHANNEL_1);
	}
	else
	{
		_Error_Handler(__FILE__, __LINE__);
	}

}

void servo_writeMS (uint8_t servo, uint8_t value)
{
	if (value > SERVO_HIGH_LIMIT || value < SERVO_LOW_LIMIT)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
	TIM_OC_InitTypeDef sConfigOC;
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = value;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (servo == SERVO_PAN)
	{
		if (HAL_TIM_PWM_ConfigChannel(SERVO_PAN_CH, &sConfigOC, TIM_CHANNEL_1)
				!= HAL_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}
		HAL_TIM_PWM_Start(SERVO_PAN_CH, TIM_CHANNEL_1);
	}
	else if (servo == SERVO_TILT) {
		if (HAL_TIM_PWM_ConfigChannel(SERVO_TILT_CH, &sConfigOC, TIM_CHANNEL_1)
				!= HAL_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}
		HAL_TIM_PWM_Start(SERVO_TILT_CH, TIM_CHANNEL_1);
	}
	else
	{
		_Error_Handler(__FILE__, __LINE__);
	}
}

void servo_writeDEG (uint8_t servo, uint8_t value)
{
	value = constrain(value, 0, 180);
	value = (uint8_t) improved_map(value, 0, 180, SERVO_LOW_LIMIT, SERVO_HIGH_LIMIT);
	servo_writeMS(servo, value);
}

uint8_t servo_posFilter(const uint8_t n, const uint8_t in, const uint8_t disabled)
{
	static uint8_t buffer[101] = {0}, pos = 0;
	uint32_t sum = 0;

	if (disabled != 0)
	{
		return in;
	}

	if ((n >= 100) || (n < 1))
	{
		return 0;
	}

	buffer[pos] = in;
	pos++;

	if (pos > n)
	{
		pos = 0;
	}

	for (uint8_t idx = 0; idx <n; idx++)
	{
		sum += buffer[idx];
	}

	return (sum/pos);
}

/* StartServoTask function */
void StartServoTask(void const * argument)
{
  /* USER CODE BEGIN StartServoTask */
	 uint32_t driveX, driveY;
	 uint32_t actX, actY;
  /* Infinite loop */
  for(;;)
  {
	  switch(currServoState){
	  case SERVOSTATE_DISABLED:
		  servo_deinit(SERVO_TILT);
		  servo_deinit(SERVO_PAN);
		  break;

	  case SERVOSTATE_INIT:
		  actX = SERVO_MIDDLE;
		  actY = SERVO_LOW_LIMIT;
		  servo_writeMS(SERVO_TILT, actY);
		  servo_writeMS(SERVO_PAN, actX);
		  currServoState = SERVOSTATE_READY;
		  break;

	  case SERVOSTATE_READY:
		  driveX = improved_map(servo_posFilter(10,x_pos,1), 0, 300,SERVO_LOW_LIMIT,SERVO_HIGH_LIMIT);
		  driveY = improved_map(servo_posFilter(10,y_pos,1), 0, 300,SERVO_LOW_LIMIT,SERVO_HIGH_LIMIT);
		  if (driveX > actX)
			  actX += SERVO_SPEED_FACT;
		  else
			  actX -= SERVO_SPEED_FACT;
		  if (driveY > actY)
			  actY += SERVO_SPEED_FACT;
		  else
			  actY -= SERVO_SPEED_FACT;

		  /*
		  actX = x_pos;
		  actY = y_pos;
		  */

		  if (actX > SERVO_HIGH_LIMIT)
				actX = SERVO_HIGH_LIMIT;
		  if (actY > SERVO_HIGH_LIMIT)
			  actY = SERVO_HIGH_LIMIT;

		  if (actX < SERVO_LOW_LIMIT)
			  actX = SERVO_LOW_LIMIT;
		  if (actY < SERVO_LOW_LIMIT)
			  actY = SERVO_LOW_LIMIT;

			servo_writeMS(SERVO_TILT, actY);
		  servo_writeMS(SERVO_PAN, actX);
		  break;

	  default:
		  Error_Handler();
		  break;

	  }
    osDelay(10);
  }
  /* USER CODE END StartServoTask */
}

