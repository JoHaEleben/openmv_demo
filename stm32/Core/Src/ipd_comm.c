/*
 * ipd_comm.c
 *
 *  Created on: 14.07.2018
 *      Author: johae
 */

#include "ipd_comm.h"

#define HOST_RX_ISR_FLAG (0x00005)
#define HOST_RX_BUFFER 1
#define END_OF_FRAME (0x40)	//TODO: Change

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART6)
    {
         osSemaphoreRelease(txSemaHandle);
    }
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

    if(huart->Instance == USART6)
    {
     osSignalSet(RxTaskHandle, HOST_RX_ISR_FLAG );
    }
}


void StartRxTask(void const * argument)
{
    // Buffer to hold message
    static uint8_t rx_buffer[HOST_RX_BUFFER] = {0};
    static uint8_t index = 0;
    //static bool in_frame = 0;
    uint8_t tmp_char = 0;
    uint8_t rx_tmp_buffer = {0};

    HAL_UART_Receive_IT( &huart6, rx_tmp_buffer, 1 );

    for(;;)
    {
     osSignalWait( HOST_RX_ISR_FLAG, osWaitForever );

     tmp_char = *( huart6.pRxBuffPtr - huart6.RxXferSize );

     // Do whatever needs to be done to the new char that just arrived
        //....
     	 WM_HWIN hItem = WM_GetDialogItem(hWin, (GUI_ID_USER + 0x07));
     	 TEXT_SetText(hItem, tmp_char);

        // Be sure to set the interrupt for the next char
        if( HAL_UART_Receive_IT( &huart6, (huart6.pRxBuffPtr - huart6.RxXferSize), 1 ) != HAL_OK)
        {
            Error_Handler();
        }
    }
}

void StartTxTask(void const * argument)
{
     osEvent evt;

     for(;;)
     {
    	 /*
          evt = osMessageGet(TxTaskHandle, osWaitForever);

          if( evt.status == osEventMessage )
          {
               message_t *msg = (message_t*) evt.value.p;

               // Take binary semaphore for UART
               osSemaphoreWait(txSemaHandle, osWaitForever);
               memcpy( (void*)&tx_buffer[4], (void*)msg->buffer, msg->msg_len );
               tx_buffer[msg->msg_len++] = END_OF_FRAME;
               HAL_UART_Transmit_DMA( &huart6, (uint8_t*)tx_buffer, msg->msg_len );
               osPoolFree( message_pool, msg );
          }
		*/
    	 uint8_t echo = 'A';
    	 HAL_UART_Transmit_IT(&huart6,&echo, 1);
    	 osDelay(500);
     }
}


