
/*
 * Protocolo.c
 *
 *  Created on: 02/10/2015
 *      Author: Pedro Henrique Fernandes
 */


#include <MAC/Protocolo.h>
#include <stdint.h>
#include <stdbool.h>
#include "../tivac.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/timer.h"
#include "utils/random.h"
#include "../hal_defs.h"
#include "Radio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "priorities.h"
#include "semphr.h"

//*****************************************************************************
//
// The stack size for the LED toggle task.
//
//*****************************************************************************
#define MASTERTASKSTACKSIZE        1024         // Stack size in words
TaskHandle_t RedeTaskHandle= NULL;
//Valor do tempo da rede no envio do último Beacon
uint32_t TRede_BEACON=0;
uint32_t TRede_TRIGGER=0;

//valor retornado pela função de inicialização do timer para resetar o timer
uint16_t Value_TimerReset=0;

//Apenas o modo master irá conhecer os endereços
uint8_t Enderecos[NMAX];
uint8_t ContMsg[NMAX];
//----------------------------------------------------------------------------------
//  Constants used in this file
//----------------------------------------------------------------------------------
#define RADIO_MODE_TX        1
#define RADIO_MODE_RX        2

#define RX_OK                0
#define RX_LENGTH_VIOLATION  1
#define RX_CRC_MISMATCH      2
#define RX_FIFO_OVERFLOW     3

//----------------------------------------------------------------------------------
//  Variables used in this file
//----------------------------------------------------------------------------------

uint8_t Led=0;

static uint32_t ProximaTransmissao=0;
//Dados da rede
static RedeConfig MinhaRede={0,0,0,0,0,0,0};

extern xSemaphoreHandle g_pTXFifoSemaphore;
TaskHandle_t RedeTaskHandle= NULL;
extern TaskHandle_t RadioTaskHandle;

typedef enum {
    OUVINDO= 0, FALANDO, DORMINDO
} States;



uint8_t data[255];
//Interrupção do timer, definido pelo TMS_REDE em Protocolo.h
//Ou, no caso do slave é definido pelo recebimento do Beacon
void TimerTimeSliceIntHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken;

	/* xHigherPriorityTaskWoken must be initialised to pdFALSE.  If calling
	xTaskNotifyFromISR() unblocks the handling task, and the priority of
	the handling task is higher than the priority of the currently running task,
	then xHigherPriorityTaskWoken will automatically get set to pdTRUE. */
	xHigherPriorityTaskWoken = pdFALSE;

	/* Unblock the handling task so the task can perform any processing necessitated
	by the interrupt.  xHandlingTask is the task's handle, which was obtained
	when the task was created.  The handling task's notification value
	is bitwise ORed with the interrupt status - ensuring bits that are already
	set are not overwritten. */
	xTaskNotifyFromISR( RedeTaskHandle,
						TIME_SLICE,
						eSetBits,
						&xHigherPriorityTaskWoken );

    //
    // Clear the timer interrupt flag.
    //
    TimerIntClear(TIMER4_BASE, TIMER_TIMA_TIMEOUT);

    /*
	Force a context switch if xHigherPriorityTaskWoken is now set to pdTRUE.
	The macro used to do this is dependent on the port and may be called
	portEND_SWITCHING_ISR. */
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

static void
MasterRedeTask(void *pvParameters)
{

	uint32_t ulNotifiedValue;
	BaseType_t xResult;

	CriaRede();

	    	while(TRUE)
	    	{

				xResult=	xTaskNotifyWait( 0x00,      /* Don't clear any notification bits on entry. */
									 0xffffffff , /* Reset the notification value to 0 on exit. */
									 &ulNotifiedValue, /* Notified value pass out in
																  ulNotifiedValue. */
									 portMAX_DELAY );  /* Block indefinitely. */

				if(xResult == pdPASS)
				{
					switch(ulNotifiedValue)
					{
					case  TIME_SLICE:

						MinhaRede.TREDE++;

						xSemaphoreTake(g_pTXFifoSemaphore, portMAX_DELAY);
							data[0]='O';
							data[1]='i';
							Push_TXFifo(BROADCAST,MinhaRede.END,TM_DADOS,data,0x02);
						xSemaphoreGive(g_pTXFifoSemaphore);


						xTaskNotify( RadioTaskHandle,
												SEND,
												eSetBits);
						break;
					case RECEIVE:
						UARTprintf("Received!\n");
						break;
					case SENT:
					 /*   xTaskNotify( RadioTaskHandle,
												LISTEN,
												eSetBits);*/
						break;
					default:
						break;
					}
				}

	    	}
}


uint32_t
MasterTaskInit(void)
{
    //
    // Create the Master task.
    //
    if(xTaskCreate(MasterRedeTask, (signed portCHAR *)"Master", MASTERTASKSTACKSIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_MASTER_TASK, &RedeTaskHandle) != pdTRUE)
    {
        return(1);
    }

    //
    // Success.
    //
    return(0);
}


void CriaRede()
{
	int i=0;

	for(i=0;i<NMAX;i++)
	{
		ContMsg[i]=0;
	}

	//Inicializa o random
	RandomAddEntropy(GetSeed());
	//Gera um valor não nulo de 8 bits
	MinhaRede.ID=(RandomSeed()%255)+1;
	RandomAddEntropy(MinhaRede.ID);
	MinhaRede.NEND=0;
	MinhaRede.TREDE=0;
	MinhaRede.TMS_TX=TMS_REDE;
	MinhaRede.TT=0;

	Value_TimerReset= InitTimer4_TimeSlice(MinhaRede.TMS_TX,&TimerTimeSliceIntHandler,configMAX_SYSCALL_INTERRUPT_PRIORITY);
}
