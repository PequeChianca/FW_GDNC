
/*
 * Protocolo.c
 *
 *  Created on: 02/10/2015
 *      Author: Pedro Henrique Fernandes
 */

#include <MAC/Protocolo.h>
#include "Radio.h"
#include <stdint.h>
#include <stdbool.h>
#include "../tivac.h"
#include "../Modules/CC1100.h"
#include "../Modules/CC1100_regs.h"
#include "../my_rf_settings.h"
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
#include "FreeRTOS.h"
#include "task.h"
#include "priorities.h"
#include "semphr.h"

xSemaphoreHandle g_pTXFifoSemaphore;


#define RADIOTASKSTACKSIZE        2048         // Stack size in words

//Contador dos pacotes
static uint8_t Packets_RxFifo=0;
static uint8_t Packets_TxFifo=0;

//Inicio das fifos
static uint8_t Inicio_TxFifo=0;
static uint8_t Inicio_RxFifo=0;

//Número de bytes a partir do inicio
static uint8_t Bytes_TxFifo=0;
static uint8_t Bytes_RxFifo=0;

static uint8_t Rede_RXFIFO[FIFO_RX_LENGTH];
static uint8_t Rede_TXFIFO[FIFO_TX_LENGTH];

static void (*GDO0_int)(void);
static void (*GDO2_int)(void);

TaskHandle_t RadioTaskHandle=NULL;
extern TaskHandle_t RedeTaskHandle;


//----------------------------------------------------------------------------------
//  void txISR(void)
//
//  DESCRIPTION:
//    This function is called (in interrupt context) every time a packet has been
//    transmitted.
//----------------------------------------------------------------------------------
void
IntGPIOb(void)
{
	volatile uint32_t Reg;

	Reg=GPIOIntStatus(CC1100_GDO_GPIO_BASE, CC1100_GDO0|CC1100_GDO2);

	switch(Reg)
	{
	case CC1100_GDO0:
		GDO0_int();
		GPIOIntClear(CC1100_GDO_GPIO_BASE, CC1100_GDO0);
		break;
	case CC1100_GDO2:
		GDO2_int();
		GPIOIntClear(CC1100_GDO_GPIO_BASE, CC1100_GDO2);
		break;
	default:
		break;
	}


}



static void
RadioTask(void *pvParameters)
{
	uint8_t* length;
	uint8_t* rssi;
	uint32_t ulNotifiedValue;
	BaseType_t xResult;
	LigaRadio();


	    	while(TRUE)
	    	{

	    		xResult=	xTaskNotifyWait( 0x00,      /* Don't clear any notification bits on entry. */
	    				 	 	 0xffffffff , /* Reset the notification value to 0 on exit. */
	    		                 &ulNotifiedValue, /* Notified value pass out in ulNotifiedValue. */
	    		                 portMAX_DELAY );  /* Block indefinitely. */

				if(xResult == pdPASS)
				{
					switch(ulNotifiedValue)
					{
					case SEND:
						xSemaphoreTake(g_pTXFifoSemaphore, portMAX_DELAY);
						Pop_TXFifo();
						xSemaphoreGive(g_pTXFifoSemaphore);
						break;
					case LISTEN:
						SetRxMode();
						break;
					case SENT_PKT:
						//TODO: Avisar alguém?
						xTaskNotify( RedeTaskHandle,
											SENT,
											eSetBits);
						break;
					case RECEIVED_PKT:
						//TODO: Avisar alguém?
						//CC1100_rxRecvPacket(Rede_RXFIFO,length,rssi );
						xTaskNotify( RedeTaskHandle,
											RECEIVE,
											eSetBits);
						break;
					case HALFFULL_TX:
						//TODO: Reiniciar o radio
						break;
					case HALFFULL_RX:
						//TODO: Reiniciar o radio
						break;
					case SLEEP:
						//TODO: Reiniciar o radio
						// Put radio to sleep
						CC1100_RfStrobe(CC1100_SPWD);
						break;
					}
				}
	    	}
}


uint32_t
RadioTaskInit(void)
{
	g_pTXFifoSemaphore = xSemaphoreCreateMutex();
    //
    // Create the LED task.
    //
    if(xTaskCreate(RadioTask, (signed portCHAR *)"Master", RADIOTASKSTACKSIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_RADIO_TASK, &RadioTaskHandle) != pdTRUE)
    {
        return(1);
    }

    //
    // Success.
    //
    return(0);
}


//Inicializa o radio CC1100
//Parametrizar as funções para utilizar o mesmo arquivo para os dois rádios
void LigaRadio()
{
	// PowerOn
	CC1100_RfResetChip();

	// Put radio to sleep
	CC1100_RfStrobe(CC1100_SPWD);

	// Setup chip with register settings from SmartRF Studio
	CC1100_RfConfig(&myRfConfig, myPaTable, myPaTableLen);

     // Additional chip configuration for this example
	 CC1100_RfWriteReg(CC1100_MCSM0,    0x18);   // Calibration from IDLE to TX/RX
	 CC1100_RfWriteReg(CC1100_MCSM1,    0x0C);   // No CCA, RX after RX, IDLE after TX
	 CC1100_RfWriteReg(CC1100_PKTCTRL0, 0x45);   // Enable data whitening and CRC
	 CC1100_RfWriteReg(CC1100_PKTCTRL1, 0x05);	 // No broadcast
	 CC1100_RfWriteReg(CC1100_ADDR,0x02);
	 //Start the radio in RX mode
	 GDO0_int = &rxFifoHalfFull;
	 //Trata pacote recebido de acordo com o protocolo
	 GDO2_int = &rxPacketRecvd;

	 CC1100_rxInit();

	 Init_GPIOInt(CC1100_GDO_GPIO_PERIPH,
	 	 	 					CC1100_GDO_GPIO_BASE,
	 	 	 					CC1100_GDO0,
	 	 	 					CC1100_INT_BASE,
	 							GPIO_FALLING_EDGE,
	 	 	 					configMAX_SYSCALL_INTERRUPT_PRIORITY,
	 	 	 					&IntGPIOb);

	 Init_GPIOInt(CC1100_GDO_GPIO_PERIPH,
	 	 					CC1100_GDO_GPIO_BASE,
	 	 					CC1100_GDO2,
	 	 					CC1100_INT_BASE,
	 						GPIO_RISING_EDGE,
	 	 					configMAX_SYSCALL_INTERRUPT_PRIORITY,
	 	 					&IntGPIOb);

	 GPIOIntClear(CC1100_GDO_GPIO_BASE, CC1100_GDO0| CC1100_GDO2);

	 resetpacketComplete();
	 resetdataPending();

	 CC1100_RfStrobe(CC1100_SRX);
}

void rxFifoHalfFull()
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
	xTaskNotifyFromISR( RadioTaskHandle,
						HALFFULL_RX,
						eSetBits,
						&xHigherPriorityTaskWoken );
	/*
	Force a context switch if xHigherPriorityTaskWoken is now set to pdTRUE.
	The macro used to do this is dependent on the port and may be called
	portEND_SWITCHING_ISR. */
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void rxPacketRecvd()
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
	xTaskNotifyFromISR( RadioTaskHandle,
						RECEIVED_PKT,
						eSetBits,
						&xHigherPriorityTaskWoken );
	/*
	Force a context switch if xHigherPriorityTaskWoken is now set to pdTRUE.
	The macro used to do this is dependent on the port and may be called
	portEND_SWITCHING_ISR. */
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void SetRxMode()
{
	//Set Rx mode
	 GPIOIntDisable(CC1100_GDO_GPIO_BASE, CC1100_GDO0|CC1100_GDO2);
	 //Start the radio in RX mode
	 GDO0_int = &rxFifoHalfFull;
	 //Trata pacote recebido de acordo com o protocolo
	 GDO2_int = &rxPacketRecvd;

	 CC1100_rxInit();

	 Init_GPIOInt(CC1100_GDO_GPIO_PERIPH,
	 	 					CC1100_GDO_GPIO_BASE,
	 	 					CC1100_GDO0,
	 	 					CC1100_INT_BASE,
							GPIO_FALLING_EDGE,
	 	 					configMAX_SYSCALL_INTERRUPT_PRIORITY,
	 	 					&IntGPIOb);

	 Init_GPIOInt(CC1100_GDO_GPIO_PERIPH,
	 					CC1100_GDO_GPIO_BASE,
	 					CC1100_GDO2,
	 					CC1100_INT_BASE,
						GPIO_RISING_EDGE,
	 					configMAX_SYSCALL_INTERRUPT_PRIORITY,
	 					&IntGPIOb);

	 GPIOIntClear(CC1100_GDO_GPIO_BASE, CC1100_GDO0| CC1100_GDO2);

	 CC1100_RfStrobe(CC1100_SRX);
}


/* Retorna se foi recebido algum pacote ou não
 * Retorna 0-> Nenhum pacote na fifo
 * Retorna 1-> Pacote recebido
 * (*Addr) do remetente
 * (*Data) Array de dados apontado por *Data
 * (*Length) Tamanho do array
 */
uint8_t Pop_RXFifo(uint8_t *Addr,uint8_t *Data,uint8_t *Length)
{
	uint8_t k=0;
	uint8_t i=0;
	uint8_t addr=0;
	uint8_t length=0;

	if(Packets_RxFifo>0)
	{
		Packets_RxFifo--;
		i=Inicio_RxFifo;
		length=Rede_RXFIFO[i];
		//Pq o próximo byte é o endereço do próprio rádio ou broadcast, então não faz
		//sentido armazenar
		i+=2;
		addr=Rede_RXFIFO[i];
		i++;

		for(k=0;k<(*Length-2);k++)
		{
			Data[k]=Rede_RXFIFO[i];
			i++;
		}

		//Atualiza o inicio da fifo e a quantidade de bytes restantes
		//caso não hajam mais pacotes, volta pro inicio da fifo(não tem pq ficar apontando pro meio)
		if(Packets_RxFifo==0)
			Inicio_RxFifo=0;
		else
			Inicio_RxFifo=i;

		Bytes_RxFifo=Bytes_RxFifo-length-1;
		*Addr=addr;
		*Length=length;

		return REDE_OK;
	}else
		return REDE_INVALIDMODE;
}


uint8_t	Push_TXFifo(uint8_t Addr,uint8_t END,uint8_t TipoMsg,uint8_t *Data,uint8_t Length)
{
	uint8_t i=0;
	uint8_t k=0;

	uint8_t Fim_TxFifo=Bytes_TxFifo+Inicio_TxFifo;

	//Vê se a msg que está sendo enviada cabe na fifo
	if(Bytes_TxFifo+Length+4<FIFO_TX_LENGTH)
	{
		//Adiona o pacote
		Packets_TxFifo++;
		//Atualiza a quantidade de bytes que existem na fifo para ser enviado
		Bytes_TxFifo+=Length+4;

		//Formato do pacote
		i=Fim_TxFifo;
		Rede_TXFIFO[i]=Length+3;
		i++;
		Rede_TXFIFO[i]=Addr;
		i++;
		Rede_TXFIFO[i]=END;
		i++;
		Rede_TXFIFO[i]=TipoMsg;
		i++;

		for(k=0;k<Length;k++)
		{
			Rede_TXFIFO[i]=Data[k];
			i++;
		}

		return RADIO_OK;
	}
	else
		return RADIO_INVALIDMODE;

}

void txPacketSent()
{
	CC1100_txPacketSent();

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
	xTaskNotifyFromISR( RadioTaskHandle,
						SENT_PKT,
						eSetBits,
						&xHigherPriorityTaskWoken );
	/*
	Force a context switch if xHigherPriorityTaskWoken is now set to pdTRUE.
	The macro used to do this is dependent on the port and may be called
	portEND_SWITCHING_ISR. */
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void txFifoHalfFull()
{
	CC1100_txFifoHalfFull();

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
	xTaskNotifyFromISR( RadioTaskHandle,
						HALFFULL_TX,
						eSetBits,
						&xHigherPriorityTaskWoken );
	/*
	Force a context switch if xHigherPriorityTaskWoken is now set to pdTRUE.
	The macro used to do this is dependent on the port and may be called
	portEND_SWITCHING_ISR. */
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

}

uint8_t Pop_TXFifo()
{
	uint8_t Data[255];
	uint8_t Length;

	uint8_t k=0;
	uint8_t i=0;

	uint8_t BytesInFifo=0;

	if(Packets_TxFifo>0)
	{
		GPIOIntDisable(CC1100_GDO_GPIO_BASE, CC1100_GDO0|CC1100_GDO2);

		// Set up interrupt on GDO0
		GDO0_int=&txFifoHalfFull;
		// Set up interrupt on GDO2
		GDO2_int=&txPacketSent;

		CC1100_txInit();

		//Init GDO0 and GDO2 interrupts
		Init_GPIOInt(CC1100_GDO_GPIO_PERIPH,
					CC1100_GDO_GPIO_BASE,
					CC1100_GDO0|CC1100_GDO2,
					CC1100_INT_BASE,
					GPIO_FALLING_EDGE,
					configMAX_SYSCALL_INTERRUPT_PRIORITY,
					&IntGPIOb);


		while(Packets_TxFifo>0)
		{
			Packets_TxFifo--;

			i=Inicio_TxFifo;
			Length=Rede_TXFIFO[i]+1;
			//Pq o próximo byte é o endereço do próprio rádio ou broadcast, então não faz
			//sentido armazenar

			for(k=0;k<Length;k++)
			{
				Data[k]=Rede_TXFIFO[i];
				i++;
			}

			//TODO: Parametrizar o delay?!
			//Delay para colocar o pacote mais pro "meio" do timeslice
			WaitUs(5);
			BytesInFifo=CC1100_txSendPacket(Data, Length);
			WaitUs(5);

			Bytes_TxFifo-=Length;

			//Se não existem mais pacotes para onde apontar, volta pro ínicio da fifo.
			if(Packets_TxFifo==0)
				Inicio_TxFifo=0;
			else
				Inicio_TxFifo=i;
		}

	}

	return BytesInFifo;

}
