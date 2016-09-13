/*
 * tivac.c
 *
 *  Created on: 28/08/2015
 *      Author: User
 */


#include "tivac.h"

#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_timer.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"

#include "inc/hw_nvic.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/timer.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"
#include "inc/tm4c123gh6pm.h"


#define DEBOUCE 100
unsigned char Button;
//*****************************************************************************
//
// This is the handler for INT_GPIOf.
//
//*****************************************************************************

void
IntGPIOf(void)
{
	uint32_t Reg;

	Reg=GPIOIntStatus(BUTTONS_GPIO_BASE, ALL_BUTTONS);

	switch(Reg)
	{
	case LEFT_BUTTON:
		WaitUs(DEBOUCE);
		Button=LEFT_BUTTON;
		break;
	case RIGHT_BUTTON:
		WaitUs(DEBOUCE);
		Button=RIGHT_BUTTON;
		break;
	case ALL_BUTTONS:
		WaitUs(DEBOUCE);
		Button=ALL_BUTTONS;
		break;
	default:
		Button=0;
		break;
	}

	GPIOIntClear(BUTTONS_GPIO_BASE, ALL_BUTTONS);
}

void WaitUs(uint16_t usec) // 5 cycles for calling
{
	uint16_t i=0;
	// In this example, the least we can wait is 12 usec:
    // ~1 us for call, 1 us for first compare and 1 us for return
	for(i=0;i<usec*80;i++);

}


/* void InitButtons()
 *  configure interrupts for
 *      SW1 connected to PF4
 *  and SW2 connected to PF0.
 *
 *  needs to configure the behavior for it in int. handler function.
 */
void InitButtons()
{
		//
	    // Enable the peripherals used by this example.
	    //
	    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	    HWREG(BUTTONS_GPIO_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	    HWREG(BUTTONS_GPIO_BASE + GPIO_O_CR) |= 0x01;
	    HWREG(BUTTONS_GPIO_BASE + GPIO_O_LOCK) = 0;

	    GPIOPinTypeGPIOInput(BUTTONS_GPIO_BASE, ALL_BUTTONS);
	    GPIOPadConfigSet(BUTTONS_GPIO_BASE, ALL_BUTTONS,
	                         GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

		GPIOIntClear(BUTTONS_GPIO_BASE, ALL_BUTTONS);
		GPIOIntTypeSet(BUTTONS_GPIO_BASE, ALL_BUTTONS, GPIO_FALLING_EDGE );	// configure the interrupts
		GPIOIntEnable(BUTTONS_GPIO_BASE, ALL_BUTTONS);
		IntEnable(INT_GPIOF);
		// Enable processor interrupts.
		IntMasterEnable();
}


void
ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}


void Init_GPIOInt(uint32_t ui32Peripheral,uint32_t ui32Port, uint8_t ui8Pins,uint32_t ui32Interrupt,uint32_t ui32IntType, uint8_t ui8Priority, void (*pfnHandler)(void))
{
	//
	// Enable the peripherals used by this example.
	//
	SysCtlPeripheralEnable(ui32Peripheral);

	GPIOPinTypeGPIOInput(ui32Port, ui8Pins);
	GPIOIntClear(ui32Port, ui8Pins);
	GPIOIntTypeSet(ui32Port, ui8Pins, ui32IntType );	// configure the interrupts
	GPIOIntRegister(ui32Port,pfnHandler);
	IntPrioritySet (ui32Interrupt, ui8Priority);
	GPIOIntEnable(ui32Port, ui8Pins);
	IntEnable(ui32Interrupt);


}

uint16_t InitTimer4_TimeSlice(uint16_t TMiliSeg,void (*pfnHandler)(void),uint8_t ui8Priority)
{
		uint32_t SClk=0;
		uint8_t Prescale=0;
		uint16_t Load=0;

        //
	    // The Timer0 peripheral must be enabled for use.
	    //
	    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER4);

	    SClk=SysCtlClockGet();

	    // 1us
	    //Prescale= SClk/100000;

	    // 3,2 us
	    Prescale= 0xff;

	    /*Tempo máximo permitido pelo timer de 16 bits
		 if (TMiliSeg>=65)
			TMiliSeg=65;


		Load=TMiliSeg*1000;
		*/

	    if (TMiliSeg>=209)
	    	TMiliSeg=209;

	    Load=(uint16_t) TMiliSeg*(312.5);

		//TimerTimeSliceIntHandler
	    // Configure Timer0B as a 16-bit periodic timer.
	    //
	    TimerConfigure(TIMER4_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC);

	    // Set Tc to 1us
	    TimerPrescaleSet(TIMER4_BASE,TIMER_A,Prescale);

	    //
	    // Set the Timer0B load value to 10ms.
	    //
	    TimerLoadSet(TIMER4_BASE, TIMER_A, Load);
	    TimerIntRegister(TIMER4_BASE,TIMER_A,pfnHandler);
	    IntPrioritySet (INT_TIMER4A_TM4C123, ui8Priority);
	    //
	    // Enable processor interrupts.
	    //
	    IntMasterEnable();

	    //
	    // Configure the Timer0B interrupt for timer timeout.
	    //
	    TimerIntEnable(TIMER4_BASE, TIMER_TIMA_TIMEOUT);

	    //
	    // Enable the Timer0B interrupt on the processor (NVIC).
	    //
	    IntEnable(INT_TIMER4A_TM4C123);

	    //
	    // Enable Timer0B.
	    //
	    TimerEnable(TIMER4_BASE, TIMER_A);

	    return Load;
}

void DisableTimer4_TimeSlice()
{

	//
	// Disable Timer0B.
	//
	TimerDisable(TIMER4_BASE, TIMER_A);

	//
	// Disable the Timer0B interrupt on the processor (NVIC).
	//
	IntDisable(INT_TIMER4A);

	//
	// Disable the Timer0B interrupt for timer timeout.
	//
	TimerIntDisable(TIMER4_BASE, TIMER_TIMA_TIMEOUT);

}

uint32_t GetSeed()
{
	uint32_t Seed;

	Seed = SysTickValueGet();
	//SysTickDisable();


	return Seed;
}
