/*
 * App.c
 *
 *  Created on: 28/04/2016
 *      Author: User
 */


#include <stdint.h>
#include <stdbool.h>
#include "../tivac.h"
#include "inc/tm4c123gh6pm.h"
#include "../hal_defs.h"
#include <driverlib/uart.h>
#include <utils/uartstdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "priorities.h"
#include "semphr.h"
#include "../Rede/Protocolo.h"

#define APPTASKSTACKSIZE        1024
TaskHandle_t AppTaskHandle= NULL;
extern TaskHandle_t MACTaskHandle;
QueueHandle_t xAppQueue;
extern QueueHandle_t TxQueue;

static void
AppTask(void *pvParameters)
{

	uint32_t ulNotifiedValue;
	BaseType_t xResult;
	uint8_t usSwitch=0;
	xMessage *xpMsg;
	xMessage xMsg;

	const TickType_t xDelay = 10 / portTICK_PERIOD_MS;


	xAppQueue = xQueueCreate( 10, sizeof( struct xMessage * ) );

	while(TRUE)
	{
		while(usSwitch==0)
		{
			UARTprintf("\n-----------FWGDNC-----------\n");
			UARTprintf("Inicio:\n Digite:\nM-Mestre da Rede \nE-Escravo da Rede \n");
			usSwitch=UARTgetc();

			if((usSwitch!='M')&&(usSwitch!='E')&&(usSwitch!='m')&&(usSwitch!='e'))
			{
				UARTprintf("\nValor invalido!!\n");
				usSwitch=0;
			}
		}

		if((usSwitch=='m')||(usSwitch=='M'))
		{
			UARTprintf("\nMestre da rede\n");
			//send signal to protocolo
			 xTaskNotify( MACTaskHandle,
									MESTRE,
									eSetBits);
			 	 	 	 	 	 	 	 while(TRUE)
									 		{
									 			if( xAppQueue != 0 )
									 			{
									 				// Receive a message on the created queue.  Block for 10 ticks if a
									 				// message is not immediately available.
									 				if( xQueueReceive( xAppQueue, (void *) &xpMsg, portMAX_DELAY ) )
									 				{
									 					// pcRxedMessage now points to the struct AMessage variable posted
									 					// by vATask.
									 					UARTprintf("USER %i [%i] :",xpMsg->ucOrigAddr,xpMsg->uwTime);
									 					UARTprintf(xpMsg->ucData);
									 					UARTprintf("\n");
									 				}
									 			}
									 		}
		}else
		{
			UARTprintf("\nEscravo da rede!!\n");
			//send signal to protocolo
						 xTaskNotify( MACTaskHandle,
												ESCRAVO,
												eSetBits);

						 while(TRUE)
						 		{
						 			if( xAppQueue != 0 )
						 			{
						 				// Receive a message on the created queue.  Block for 10 ticks if a
						 				// message is not immediately available.
						 				if( xQueueReceive( xAppQueue, (void *) &xpMsg, xDelay ) )
						 				{
						 					// pcRxedMessage now points to the struct AMessage variable posted
						 					// by vATask.
						 					UARTprintf("USER %i [%i] :",xpMsg->ucOrigAddr,xpMsg->uwTime);
						 					UARTprintf(xpMsg->ucData);
						 					UARTprintf("\n");
						 				}else
						 				{
						 					UARTprintf("Envie sua mensagem:");
						 					xMsg.ucLenght=UARTgets(xMsg.ucData,MAX_MSGLEN);
						 					xMsg.ucDestAddr=BROADCAST;
						 					xpMsg = &xMsg;
						 					xQueueSend(TxQueue, (void * ) &xpMsg, (TickType_t ) 0);
						 				}
						 			}
						 		}
		}


		//xResult=	xTaskNotifyWait( 0x00,      /* Don't clear any notification bits on entry. */
		//					 0xffffffff , /* Reset the notification value to 0 on exit. */
		//					 &ulNotifiedValue, /* Notified value pass out in ulNotifiedValue. */
		//					 portMAX_DELAY );  /* Block indefinitely. */


	}
}

uint32_t
AppTaskInit(void)
{
    //
    // Create the Master task.
    //
    if(xTaskCreate(AppTask, (signed portCHAR *)"App", APPTASKSTACKSIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_APP_TASK, &AppTaskHandle) != pdTRUE)
    {
        return(1);
    }

    //
    // Success.
    //
    return(0);
}
