/*
 * SPI.c
 *
 *  Created on: 20/08/2015
 *      Author: User
 */

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "SPI.h"

#include "inc/tm4c123gh6pm.h"



void SPI0_Init(uint32_t ui32BitRate, uint32_t ui32DataWidth)
{

		uint32_t pui32DataRx;

		//
	    // The SSI0 peripheral must be enabled for use.
	    //
	    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);

	    //
	    // For this example SSI0 is used with PortA[5:2].  The actual port and
	    // pins used may be different on your part, consult the data sheet for
	    // more information.  GPIO port A needs to be enabled so these pins can
	    // be used.
	    // TODO: change this to whichever GPIO port you are using.
	    //
	    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	    //
	    // Configure the pin muxing for SSI0 functions on port A2, A3, A4, and A5.
	    // This step is not necessary if your part does not support pin muxing.
	    // TODO: change this to select the port/pin you are using.
	    //
	    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
	    GPIOPinConfigure(GPIO_PA3_SSI0FSS);
	    GPIOPinConfigure(GPIO_PA4_SSI0RX);
	    GPIOPinConfigure(GPIO_PA5_SSI0TX);

	    //
	    // Configure the GPIO settings for the SSI pins.  This function also gives
	    // control of these pins to the SSI hardware.  Consult the data sheet to
	    // see which functions are allocated per pin.
	    // The pins are assigned as follows:
	    //      PA5 - SSI0Tx
	    //      PA4 - SSI0Rx
	    //      PA3 - SSI0Fss
	    //      PA2 - SSI0CLK
	    // TODO: change this to select the port/pin you are using.
	    //
	    GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_3 |
	                   GPIO_PIN_2);

	    //
	    // Configure and enable the SSI port for TI master mode.  Use SSI0, system
	    // clock supply, master mode, 1MHz SSI frequency, and 8-bit data.
	    //
	    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_TI,
	                       SSI_MODE_MASTER, ui32BitRate, ui32DataWidth);

	    //
	    // Enable the SSI0 module.
	    //
	    SSIEnable(SSI0_BASE);

	    //
	    // Read any residual data from the SSI port.  This makes sure the receive
	    // FIFOs are empty, so we don't read any unwanted junk.  This is done here
	    // because the TI SSI mode is full-duplex, which allows you to send and
	    // receive at the same time.  The SSIDataGetNonBlocking function returns
	    // "true" when data was returned, and "false" when no data was returned.
	    // The "non-blocking" function checks if there is any data in the receive
	    // FIFO and does not "hang" if there isn't.
	    //
	    while(SSIDataGetNonBlocking(SSI0_BASE, &pui32DataRx))
	    {
	    }

}

//----------------------------------------------------------------------------------
//  void halWrite(uint8 addr, const uint8 *buffer, uint16 length)
//
//  DESCRIPTION:
//    Write data to device, starting at internal device address "addr".
//    The device will increment the address internally for every new byte
//    that is written. For single byte write, set length to 1.
//----------------------------------------------------------------------------------
uint8_t SPI0_Write(uint8_t addr, const uint8_t* data, uint16_t length)
{
    uint16_t i;
    uint8_t rc;

    SSIDataPut(SSI0_BASE,addr);

	while(SSIBusy(SSI0_BASE));

    while(SSIDataGetNonBlocking(SSI0_BASE, &rc));

    for (i = 0; i < length; i++)
    {
		SSIDataPut(SSI0_BASE,data[i]);

		while(SSIBusy(SSI0_BASE));

    }


    return(rc);

}

//----------------------------------------------------------------------------------
//  uint8 halRead(uint8 addr, uint8* data, uint16 length)
//
//  DESCRIPTION:
//    Read data from device, starting at internal device address "addr".
//    The device will increment the address internally for every new byte
//    that is read. Note that the master device needs to write a dummy byte
//    (in this case 0) for every new byte in order to generate the clock to
//    clock out the data. For single byte read, set length to 1.
//----------------------------------------------------------------------------------
uint8_t SPI0_Read(uint8_t addr, uint8_t* data, uint16_t length)
{
    uint16_t i;
    uint8_t rc;

   SSIDataPut(SSI0_BASE,addr);

	while(SSIBusy(SSI0_BASE));

    while(SSIDataGetNonBlocking(SSI0_BASE, &rc));

    for (i = 0; i < length; i++)
    {
		SSIDataPut(SSI0_BASE,0);

		while(SSIBusy(SSI0_BASE));

		SSIDataGetNonBlocking(SSI0_BASE, &data[i]);

    }

    return(rc);
}


//----------------------------------------------------------------------------------
//  uint8 halSpiStrobe(uint8 cmd)
//
//  DESCRIPTION:
//    Special write function, writing only one byte (cmd) to the device.
//----------------------------------------------------------------------------------
uint8_t SPI0_Strobe(uint8_t cmd)
{
    uint8_t rc;

    SSIDataPut(SSI0_BASE,cmd);

	while(SSIBusy(SSI0_BASE));

    while(SSIDataGetNonBlocking(SSI0_BASE, &rc));

    return(rc);
}


void SPI1_Init(uint32_t ui32BitRate, uint32_t ui32DataWidth)
{
	uint32_t pui32DataRx;

			//
		    // The SSI0 peripheral must be enabled for use.
		    //
		    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI1);

		    //
		    // For this example SSI0 is used with PortF[0:3].  The actual port and
		    // pins used may be different on your part, consult the data sheet for
		    // more information.  GPIO port F needs to be enabled so these pins can
		    // be used.
		    // TODO: change this to whichever GPIO port you are using.
		    //
		    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

		    //
		    // Configure the pin muxing for SSI1 functions on port F0, F1, F2 and F3.
		    // This step is not necessary if your part does not support pin muxing.
		    // TODO: change this to select the port/pin you are using.
		    //
		    GPIOPinConfigure(GPIO_PF0_SSI1RX);
		    GPIOPinConfigure(GPIO_PF1_SSI1TX);
		    GPIOPinConfigure(GPIO_PF2_SSI1CLK);
		    GPIOPinConfigure(GPIO_PF3_SSI1FSS);


		    //
		    // Configure the GPIO settings for the SSI pins.  This function also gives
		    // control of these pins to the SSI hardware.  Consult the data sheet to
		    // see which functions are allocated per pin.
		    // The pins are assigned as follows:
		    //      PF0 - SSI0Rx
		    //      PF1 - SSI0Tx
		    //      PF2 - SSI0CLK
		    //      PF3 - SSI0FSS
		    // TODO: change this to select the port/pin you are using.
		    //
		    GPIOPinTypeSSI(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |
		                   GPIO_PIN_3);

		    //
		    // Configure and enable the SSI port for TI master mode.  Use SSIx, system
		    // clock supply, master mode.
		    //
		    SSIConfigSetExpClk(SSI1_BASE, SysCtlClockGet(), SSI_FRF_TI,
		                       SSI_MODE_MASTER, ui32BitRate, ui32DataWidth);

		    //
		    // Enable the SSIx module.
		    //
		    SSIEnable(SSI1_BASE);

		    //
		    // Read any residual data from the SSI port.  This makes sure the receive
		    // FIFOs are empty, so we don't read any unwanted junk.  This is done here
		    // because the TI SSI mode is full-duplex, which allows you to send and
		    // receive at the same time.  The SSIDataGetNonBlocking function returns
		    // "true" when data was returned, and "false" when no data was returned.
		    // The "non-blocking" function checks if there is any data in the receive
		    // FIFO and does not "hang" if there isn't.
		    //
		    while(SSIDataGetNonBlocking(SSI1_BASE, &pui32DataRx))
		    {
		    }

}
//----------------------------------------------------------------------------------
//  void halWrite(uint8 addr, const uint8 *buffer, uint16 length)
//
//  DESCRIPTION:
//    Write data to device, starting at internal device address "addr".
//    The device will increment the address internally for every new byte
//    that is written. For single byte write, set length to 1.
//----------------------------------------------------------------------------------
uint8_t SPI1_Write(uint8_t addr, const uint8_t* data, uint16_t length)
{
    uint16_t i;
    uint8_t rc;

    SSIDataPut(SSI1_BASE,addr);

	while(SSIBusy(SSI1_BASE));

    while(SSIDataGetNonBlocking(SSI1_BASE, &rc));

    for (i = 0; i < length; i++)
    {
		SSIDataPut(SSI1_BASE,data[i]);

		while(SSIBusy(SSI1_BASE));

    }


    return(rc);

}

//----------------------------------------------------------------------------------
//  uint8 halRead(uint8 addr, uint8* data, uint16 length)
//
//  DESCRIPTION:
//    Read data from device, starting at internal device address "addr".
//    The device will increment the address internally for every new byte
//    that is read. Note that the master device needs to write a dummy byte
//    (in this case 0) for every new byte in order to generate the clock to
//    clock out the data. For single byte read, set length to 1.
//----------------------------------------------------------------------------------
uint8_t SPI1_Read(uint8_t addr, uint8_t* data, uint16_t length)
{
    uint16_t i;
    uint8_t rc;

   SSIDataPut(SSI1_BASE,addr);

	while(SSIBusy(SSI1_BASE));

    while(SSIDataGetNonBlocking(SSI1_BASE, &rc));

    for (i = 0; i < length; i++)
    {
		SSIDataPut(SSI1_BASE,0);

		while(SSIBusy(SSI1_BASE));

		SSIDataGetNonBlocking(SSI1_BASE, &data[i]);

    }

    return(rc);
}


//----------------------------------------------------------------------------------
//  uint8 halSpiStrobe(uint8 cmd)
//
//  DESCRIPTION:
//    Special write function, writing only one byte (cmd) to the device.
//----------------------------------------------------------------------------------
uint8_t SPI1_Strobe(uint8_t cmd)
{
    uint8_t rc;

    SSIDataPut(SSI1_BASE,cmd);

	while(SSIBusy(SSI1_BASE));

    while(SSIDataGetNonBlocking(SSI1_BASE, &rc));

    return(rc);
}


void SPI2_Init(uint32_t ui32BitRate, uint32_t ui32DataWidth)
{
	uint32_t pui32DataRx;

				//
			    // The SSI0 peripheral must be enabled for use.
			    //
			    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);

			    //
			    // For this example SSIx is used with PortB[4:7].  The actual port and
			    // pins used may be different on your part, consult the data sheet for
			    // more information.  GPIO port F needs to be enabled so these pins can
			    // be used.
			    // TODO: change this to whichever GPIO port you are using.
			    //
			    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

			    //
			    // Configure the pin muxing for SSI1 functions on port F0, F1, F2 and F3.
			    // This step is not necessary if your part does not support pin muxing.
			    // TODO: change this to select the port/pin you are using.
			    //
			    GPIOPinConfigure(GPIO_PB4_SSI2CLK);
			    GPIOPinConfigure(GPIO_PB5_SSI2FSS);
			    GPIOPinConfigure(GPIO_PB6_SSI2RX);
			    GPIOPinConfigure(GPIO_PB7_SSI2TX);

			    //
			    // Configure the GPIO settings for the SSI pins.  This function also gives
			    // control of these pins to the SSI hardware.  Consult the data sheet to
			    // see which functions are allocated per pin.
			    // The pins are assigned as follows:
			    //      PB4 - SSI0Tx
			    //      PB5 - SSI0Rx
			    //      PB6 - SSI0Fss
			    //      PB7 - SSI0CLK
			    // TODO: change this to select the port/pin you are using.
			    //
			    GPIOPinTypeSSI(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 |
			                   GPIO_PIN_7);

			    //
			    // Configure and enable the SSI port for TI master mode.  Use SSIx, system
			    // clock supply, master mode.
			    //
			    SSIConfigSetExpClk(SSI2_BASE, SysCtlClockGet(), SSI_FRF_TI,
			                       SSI_MODE_MASTER, ui32BitRate, ui32DataWidth);

			    //
			    // Enable the SSIx module.
			    //
			    SSIEnable(SSI2_BASE);

			    //
			    // Read any residual data from the SSI port.  This makes sure the receive
			    // FIFOs are empty, so we don't read any unwanted junk.  This is done here
			    // because the TI SSI mode is full-duplex, which allows you to send and
			    // receive at the same time.  The SSIDataGetNonBlocking function returns
			    // "true" when data was returned, and "false" when no data was returned.
			    // The "non-blocking" function checks if there is any data in the receive
			    // FIFO and does not "hang" if there isn't.
			    //
			    while(SSIDataGetNonBlocking(SSI2_BASE, &pui32DataRx))
			    {
			    }
}

//----------------------------------------------------------------------------------
//  void halWrite(uint8 addr, const uint8 *buffer, uint16 length)
//
//  DESCRIPTION:
//    Write data to device, starting at internal device address "addr".
//    The device will increment the address internally for every new byte
//    that is written. For single byte write, set length to 1.
//----------------------------------------------------------------------------------
uint8_t SPI2_Write(uint8_t addr, const uint8_t* data, uint16_t length)
{
    uint16_t i;
    uint8_t rc;

    SSIDataPut(SSI2_BASE,addr);

	while(SSIBusy(SSI2_BASE));

    while(SSIDataGetNonBlocking(SSI2_BASE, &rc));

    for (i = 0; i < length; i++)
    {
		SSIDataPut(SSI2_BASE,data[i]);

		while(SSIBusy(SSI2_BASE));

    }


    return(rc);

}

//----------------------------------------------------------------------------------
//  uint8 halRead(uint8 addr, uint8* data, uint16 length)
//
//  DESCRIPTION:
//    Read data from device, starting at internal device address "addr".
//    The device will increment the address internally for every new byte
//    that is read. Note that the master device needs to write a dummy byte
//    (in this case 0) for every new byte in order to generate the clock to
//    clock out the data. For single byte read, set length to 1.
//----------------------------------------------------------------------------------
uint8_t SPI2_Read(uint8_t addr, uint8_t* data, uint16_t length)
{
    uint16_t i;
    uint8_t rc;

   SSIDataPut(SSI2_BASE,addr);

	while(SSIBusy(SSI2_BASE));

    while(SSIDataGetNonBlocking(SSI2_BASE, &rc));

    for (i = 0; i < length; i++)
    {
		SSIDataPut(SSI2_BASE,0);

		while(SSIBusy(SSI2_BASE));

		SSIDataGetNonBlocking(SSI2_BASE, &data[i]);

    }

    return(rc);
}


//----------------------------------------------------------------------------------
//  uint8 halSpiStrobe(uint8 cmd)
//
//  DESCRIPTION:
//    Special write function, writing only one byte (cmd) to the device.
//----------------------------------------------------------------------------------
uint8_t SPI2_Strobe(uint8_t cmd)
{
    uint8_t rc;

    SSIDataPut(SSI2_BASE,cmd);

	while(SSIBusy(SSI2_BASE));

    while(SSIDataGetNonBlocking(SSI2_BASE, &rc));

    return(rc);
}


void SPI3_Init(uint32_t ui32BitRate, uint32_t ui32DataWidth)
{
	uint32_t pui32DataRx;

					//
				    // The SSI0 peripheral must be enabled for use.
				    //
				    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI3);

				    //
				    // For this example SSIx is used with PortB[4:7].  The actual port and
				    // pins used may be different on your part, consult the data sheet for
				    // more information.  GPIO port F needs to be enabled so these pins can
				    // be used.
				    // TODO: change this to whichever GPIO port you are using.
				    //
				    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

				    //
				    // Configure the pin muxing for SSI1 functions on port F0, F1, F2 and F3.
				    // This step is not necessary if your part does not support pin muxing.
				    // TODO: change this to select the port/pin you are using.
				    //
				    GPIOPinConfigure(GPIO_PD0_SSI3CLK);
				    //GPIOPinConfigure(GPIO_PD1_SSI3FSS);
				    GPIOPinConfigure(GPIO_PD2_SSI3RX);
				    GPIOPinConfigure(GPIO_PD3_SSI3TX);

				    //
				    // Configure the GPIO settings for the SSI pins.  This function also gives
				    // control of these pins to the SSI hardware.  Consult the data sheet to
				    // see which functions are allocated per pin.
				    // The pins are assigned as follows:
				    //      PD3 - SSI0Tx
				    //      PD2 - SSI0Rx
				    //      PD1 - SSI0Fss
				    //      PD0 - SSI0CLK
				    // TODO: change this to select the port/pin you are using.
				    //
				    GPIOPinTypeSSI(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_2 |
				                   GPIO_PIN_3);

				    //
				    // Configure and enable the SSI port for TI master mode.  Use SSIx, system
				    // clock supply, master mode.
				    //
				    SSIConfigSetExpClk(SSI3_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0 ,
				                       SSI_MODE_MASTER, ui32BitRate, ui32DataWidth);

				    //
				    // Enable the SSIx module.
				    //
				    SSIEnable(SSI3_BASE);

				    //
				    // Read any residual data from the SSI port.  This makes sure the receive
				    // FIFOs are empty, so we don't read any unwanted junk.  This is done here
				    // because the TI SSI mode is full-duplex, which allows you to send and
				    // receive at the same time.  The SSIDataGetNonBlocking function returns
				    // "true" when data was returned, and "false" when no data was returned.
				    // The "non-blocking" function checks if there is any data in the receive
				    // FIFO and does not "hang" if there isn't.
				    //
				    while(SSIDataGetNonBlocking(SSI3_BASE, &pui32DataRx))
				    {
				    }
}

//----------------------------------------------------------------------------------
//  void halWrite(uint8 addr, const uint8 *buffer, uint16 length)
//
//  DESCRIPTION:
//    Write data to device, starting at internal device address "addr".
//    The device will increment the address internally for every new byte
//    that is written. For single byte write, set length to 1.
//----------------------------------------------------------------------------------
uint32_t SPI3_Write(uint8_t addr, const uint8_t* data, uint16_t length)
{
    uint16_t i;
    uint32_t rc;
    uint32_t pui32DataRx;

	GPIO_PORTD_DATA_R &= ~(GPIO_PIN_1);

    SSIDataPutNonBlocking(SSI3_BASE,addr);

    while(SSIBusy(SSI3_BASE));

	SSIDataGetNonBlocking(SSI3_BASE, &rc);

    for (i = 0; i < length; i++)
    {
		SSIDataPutNonBlocking(SSI3_BASE,data[i]);

		while(SSIBusy(SSI3_BASE));

    }

    while(SSIDataGetNonBlocking(SSI3_BASE, &pui32DataRx))
	{
	}

	GPIO_PORTD_DATA_R |= GPIO_PIN_1;
    return(rc);

}

//----------------------------------------------------------------------------------
//  uint8 halRead(uint8 addr, uint8* data, uint16 length)
//
//  DESCRIPTION:
//    Read data from device, starting at internal device address "addr".
//    The device will increment the address internally for every new byte
//    that is read. Note that the master device needs to write a dummy byte
//    (in this case 0) for every new byte in order to generate the clock to
//    clock out the data. For single byte read, set length to 1.
//----------------------------------------------------------------------------------
uint32_t SPI3_Read(uint8_t addr, uint8_t* data, uint16_t length)
{
    uint16_t i;
    uint32_t rc;


	GPIO_PORTD_DATA_R &= ~(GPIO_PIN_1);
    SSIDataPutNonBlocking(SSI3_BASE,addr);

	while(SSIBusy(SSI3_BASE));

	while(SSIDataGetNonBlocking(SSI3_BASE, &rc));


    for (i = 0; i < length; i++)
    {
		SSIDataPutNonBlocking(SSI3_BASE,0);

		while(SSIBusy(SSI3_BASE));

		while(SSIDataGetNonBlocking(SSI3_BASE, &data[i]));

    }

	GPIO_PORTD_DATA_R |= GPIO_PIN_1;
    return(rc);
}


//----------------------------------------------------------------------------------
//  uint8 halSpiStrobe(uint8 cmd)
//
//  DESCRIPTION:
//    Special write function, writing only one byte (cmd) to the device.
//----------------------------------------------------------------------------------
uint32_t SPI3_Strobe(uint8_t cmd)
{
    uint32_t rc;

	GPIO_PORTD_DATA_R &= ~(GPIO_PIN_1);
    SSIDataPutNonBlocking(SSI3_BASE,cmd);

	while(SSIBusy(SSI3_BASE));

	while(SSIDataGetNonBlocking(SSI3_BASE, &rc));

	GPIO_PORTD_DATA_R |= GPIO_PIN_1;

    return(rc);
}


