/*
 * tivac.h
 *
 *  Created on: 28/08/2015
 *      Author: User
 */

#ifndef FREERTOS_DEMO_V7_0_TIVAC_H_
#define FREERTOS_DEMO_V7_0_TIVAC_H_

#include <stdint.h>
#include "inc/hw_timer.h"
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_types.h"

#define BUTTONS_GPIO_PERIPH     SYSCTL_PERIPH_GPIOF
#define BUTTONS_GPIO_BASE       GPIO_PORTF_BASE

#define NUM_BUTTONS             2
#define LEFT_BUTTON             GPIO_PIN_4
#define RIGHT_BUTTON            GPIO_PIN_0

#define LED_BASE				GPIO_PORTF_BASE
#define LED_GREEN				GPIO_PIN_3
#define LED_BLUE				GPIO_PIN_2
#define LED_RED					GPIO_PIN_1

#define ALL_BUTTONS            (LEFT_BUTTON | RIGHT_BUTTON)

#define LED_OFF(x)			   GPIO_PORTF_DATA_R &= (~x)
#define LED_ON(x)			   GPIO_PORTF_DATA_R |= (x)
#define LED_TOGGLE(x)		   GPIO_PORTF_DATA_R ^= (x)
#define RESETTIME(x)		   HWREG(TIMER4_BASE + TIMER_O_TAILR) = x

extern unsigned char Button;

void WaitUs(uint16_t usec);
void InitButtons();
void ConfigureUART(void);
void Init_GPIOInt(uint32_t ui32Peripheral,uint32_t ui32Port, uint8_t ui8Pins,uint32_t ui32Interrupt,uint32_t ui32IntType, uint8_t ui8Priority, void (*pfnHandler)(void));
uint16_t InitTimer4_TimeSlice(uint16_t TMiliSeg, void (*pfnHandler)(void), uint8_t ui8Priority);
void DisableTimer4_TimeSlice();
uint32_t GetSeed();


#endif /* FREERTOS_DEMO_V7_0_TIVAC_H_ */
