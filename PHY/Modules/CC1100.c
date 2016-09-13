/*
 * CC1100.c
 *
 *  Created on: 19/08/2015
 *      Author: Pedro Henrique Fernandes
 */
#include <stdbool.h>
#include <stdint.h>

#include "cc1100_regs.h"
#include "CC1100.h"
#include <tivac.h>
#include <SPI.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "inc/tm4c123gh6pm.h"
#include <hal_defs.h>

//----------------------------------------------------------------------------------
//  Variables used in this file
//----------------------------------------------------------------------------------
static volatile uint8_t dataPending;
//extern static uint8_t packetComplete;
static volatile uint8_t  txStrobeNeeded   = TRUE;
static volatile uint8_t  nPacketsPending  = 0;
static volatile uint16_t nBytesInFifo     = 0;

void CC1100_RfPowerOn()
{
	CC1100_RfResetChip();
}


void CC1100_RfResetChip(void)
{
	uint32_t pui32DataRx;

    SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOD;

    pui32DataRx=SYSCTL_RCGC2_R;

    GPIO_PORTD_DIR_R = GPIO_PIN_1;
    GPIO_PORTD_DEN_R = GPIO_PIN_1;

	// Toggle chip select signal
	GPIO_PORTD_DATA_R |= GPIO_PIN_1;
	WaitUs(30);
	GPIO_PORTD_DATA_R &= ~(GPIO_PIN_1);
	WaitUs(30);
	GPIO_PORTD_DATA_R |= GPIO_PIN_1;
	WaitUs(45);


	SPI3_Init(1000000, 8);

	GPIO_PORTD_DATA_R &= ~(GPIO_PIN_1);
    while(SSIBusy(SSI3_BASE))
    {
    }

    SSIDataPut(SSI3_BASE,CC1100_SRES);

    while(SSIBusy(SSI3_BASE))
    {
    }
	GPIO_PORTD_DATA_R |= GPIO_PIN_1;

    // Wait for chip to finish internal reset
    while(SSIDataGetNonBlocking(SSI3_BASE, &pui32DataRx))
	{
	}

}



//----------------------------------------------------------------------------------
//  void halRfConfig(const HAL_RF_CONFIG* rfConfig, const uint8* rfPaTable, uint8 rfPaTableLen)
//
//  DESCRIPTION:
//    Used to configure the CC1100/CC1100 registers with exported register
//    settings from SmartRF Studio.
//
//  ARGUMENTS:
//    rfConfig     - register settings (as exported from SmartRF Studio)
//    rfPaTable    - array of PA table values (from SmartRF Studio)
//    rfPaTableLen - length of PA table
//
//----------------------------------------------------------------------------------
void CC1100_RfConfig(const HAL_RF_CONFIG* rfConfig, const uint8_t* rfPaTable, uint8_t rfPaTableLen)
{
	//
	// Rf settings for CC1101
	//
	//
	// Rf settings for CC1101
	//
	CC1100_RfWriteReg(CC1100_IOCFG0,rfConfig->iocfg0);           //GDO0 Output Pin Configuration
	CC1100_RfWriteReg(CC1100_FIFOTHR,rfConfig->fifothr);         //RX FIFO and TX FIFO Thresholds
	CC1100_RfWriteReg(CC1100_PKTCTRL0,rfConfig->pktctrl0);       //Packet Automation Control
	CC1100_RfWriteReg(CC1100_FSCTRL1,rfConfig->fsctrl1);         //Frequency Synthesizer Control
	CC1100_RfWriteReg(CC1100_FREQ2,rfConfig->freq2);             //Frequency Control Word, High Byte
	CC1100_RfWriteReg(CC1100_FREQ1,rfConfig->freq1);             //Frequency Control Word, Middle Byte
	CC1100_RfWriteReg(CC1100_FREQ0,rfConfig->freq0);             //Frequency Control Word, Low Byte
	CC1100_RfWriteReg(CC1100_MDMCFG4,rfConfig->mdmcfg4);         //Modem Configuration
	CC1100_RfWriteReg(CC1100_MDMCFG3,rfConfig->mdmcfg3);         //Modem Configuration
	CC1100_RfWriteReg(CC1100_MDMCFG2,rfConfig->mdmcfg2);         //Modem Configuration
	CC1100_RfWriteReg(CC1100_DEVIATN,rfConfig->deviatn);         //Modem Deviation Setting
	CC1100_RfWriteReg(CC1100_MCSM0,rfConfig->mcsm0);             //Main Radio Control State Machine Configuration
	CC1100_RfWriteReg(CC1100_FOCCFG,rfConfig->foccfg);           //Frequency Offset Compensation Configuration
	CC1100_RfWriteReg(CC1100_WORCTRL,rfConfig->worctrl);         //Wake On Radio Control
	CC1100_RfWriteReg(CC1100_FSCAL3,rfConfig->fscal3);           //Frequency Synthesizer Calibration
	CC1100_RfWriteReg(CC1100_FSCAL2,rfConfig->fscal2);           //Frequency Synthesizer Calibration
	CC1100_RfWriteReg(CC1100_FSCAL1,rfConfig->fscal1);           //Frequency Synthesizer Calibration
	CC1100_RfWriteReg(CC1100_FSCAL0,rfConfig->fscal0);           //Frequency Synthesizer Calibration
	CC1100_RfWriteReg(CC1100_TEST2,rfConfig->test2);             //Various Test Settings
	CC1100_RfWriteReg(CC1100_TEST1,rfConfig->test1);             //Various Test Settings
	CC1100_RfWriteReg(CC1100_TEST0,rfConfig->test0);             //Various Test Settings

    SPI3_Write(CC1100_PATABLE | CC1100_WRITE_BURST, rfPaTable, rfPaTableLen);
}


//----------------------------------------------------------------------------------
//  void  halRfBurstConfig(const HAL_RF_BURST_CONFIG rfConfig, const uint8* rfPaTable, uint8 rfPaTableLen)
//
//  DESCRIPTION:
//    Used to configure all of the CC1100/CC1100 registers in one burst write.
//
//  ARGUMENTS:
//    rfConfig     - register settings
//    rfPaTable    - array of PA table values (from SmartRF Studio)
//    rfPaTableLen - length of PA table
//
//----------------------------------------------------------------------------------
void  CC1100_RfBurstConfig(const HAL_RF_BURST_CONFIG rfConfig, const uint8_t* rfPaTable, uint8_t rfPaTableLen)
{
	SPI3_Write(CC1100_IOCFG2  | CC1100_WRITE_BURST, rfConfig, sizeof(rfConfig));
	SPI3_Write(CC1100_PATABLE | CC1100_WRITE_BURST, rfPaTable, rfPaTableLen);
}

//----------------------------------------------------------------------------------
//  uint8 halRfGetChipId(void)
//----------------------------------------------------------------------------------
uint8_t CC1100_RfGetChipId(void)
{
    return(CC1100_RfReadStatusReg(CC1100_PARTNUM));
}

//----------------------------------------------------------------------------------
//  uint8 halRfGetChipVer(void)
//----------------------------------------------------------------------------------
uint8_t CC1100_RfGetChipVer(void)
{
    return(CC1100_RfReadStatusReg(CC1100_VERSION));
}

//----------------------------------------------------------------------------------
//  HAL_RF_STATUS halRfStrobe(uint8 cmd)
//----------------------------------------------------------------------------------
HAL_RF_STATUS CC1100_RfStrobe(uint8_t cmd)
{
    return(SPI3_Strobe(cmd));
}

//----------------------------------------------------------------------------------
//  uint8 halRfReadStatusReg(uint8 addr)
//
//  NOTE:
//      When reading a status register over the SPI interface while the register
//      is updated by the radio hardware, there is a small, but finite, probability
//      that the result is corrupt. The CC1100 and CC1100 errata notes explain the
//      problem and propose several workarounds.
//
//----------------------------------------------------------------------------------
uint8_t CC1100_RfReadStatusReg(uint8_t addr)
{
    uint8_t reg;
    SPI3_Read(addr | CC1100_READ_BURST, &reg, 1);
    return(reg);
}

//----------------------------------------------------------------------------------
//  uint8 halRfReadReg(uint8 addr)
//----------------------------------------------------------------------------------
uint8_t CC1100_RfReadReg(uint8_t addr)
{
    uint8_t reg;
    SPI3_Read(addr | CC1100_READ_SINGLE, &reg, 1);
    return(reg);
}

//----------------------------------------------------------------------------------
//  HAL_RF_STATUS halRfWriteReg(uint8 addr, uint8 data)
//----------------------------------------------------------------------------------
HAL_RF_STATUS CC1100_RfWriteReg(uint8_t addr, uint8_t data)
{
    uint8_t rc;
    rc =  SPI3_Write(addr, &data, 1);
    return(rc);
}

//----------------------------------------------------------------------------------
//  HAL_RF_STATUS halRfWriteFifo(uint8* data, uint8 length)
//----------------------------------------------------------------------------------
HAL_RF_STATUS CC1100_RfWriteFifo(const uint8_t* data, uint8_t length)
{
    return(SPI3_Write(CC1100_TXFIFO | CC1100_WRITE_BURST, data, length));
}

//----------------------------------------------------------------------------------
//  HAL_RF_STATUS halRfReadFifo(uint8* data, uint8 length)
//----------------------------------------------------------------------------------
HAL_RF_STATUS CC1100_RfReadFifo(uint8_t* data, uint8_t length)
{
    return(SPI3_Read(CC1100_RXFIFO | CC1100_READ_BURST, data, length));
}

//----------------------------------------------------------------------------------
//  uint8 halRfGetTxStatus(void)
//
//  DESCRIPTION:
//      This function transmits a No Operation Strobe (SNOP) to get the status of
//      the radio and the number of free bytes in the TX FIFO
//
//      Status byte:
//
//      ---------------------------------------------------------------------------
//      |          |            |                                                 |
//      | CHIP_RDY | STATE[2:0] | FIFO_BYTES_AVAILABLE (free bytes in the TX FIFO |
//      |          |            |                                                 |
//      ---------------------------------------------------------------------------
//
//  NOTE:
//      When reading a status register over the SPI interface while the register
//      is updated by the radio hardware, there is a small, but finite, probability
//      that the result is corrupt. This also applies to the chip status byte. The
//      CC1100 and CC1100 errata notes explain the problem and propose several
//      workarounds.
//
//----------------------------------------------------------------------------------
HAL_RF_STATUS CC1100_RfGetTxStatus(void)
{
    return(SPI3_Strobe(CC1100_SNOP));
}

//----------------------------------------------------------------------------------
//  uint8 halRfGetRxStatus(void)
//
//  DESCRIPTION:
//      This function transmits a No Operation Strobe (SNOP) with the read bit set
//      to get the status of the radio and the number of available bytes in the RX
//      FIFO.
//
//      Status byte:
//
//      --------------------------------------------------------------------------------
//      |          |            |                                                      |
//      | CHIP_RDY | STATE[2:0] | FIFO_BYTES_AVAILABLE (available bytes in the RX FIFO |
//      |          |            |                                                      |
//      --------------------------------------------------------------------------------
//
//  NOTE:
//      When reading a status register over the SPI interface while the register
//      is updated by the radio hardware, there is a small, but finite, probability
//      that the result is corrupt. This also applies to the chip status byte. The
//      CC1100 and CC1100 errata notes explain the problem and propose several
//      workarounds.
//
//----------------------------------------------------------------------------------
HAL_RF_STATUS CC1100_RfGetRxStatus(void)
{
    return(SPI3_Strobe(CC1100_SNOP | CC1100_READ_SINGLE));
}


//----------------------------------------------------------------------------------
//  void rxFifoHalfFull(void)
//
//  DESCRIPTION:
//    This function is called when the FIFO Threshold signal is asserted, indicating
//    that the FIFO (in this example) is half full. Set a flag indicating that there
//    is data in the FIFO.
//----------------------------------------------------------------------------------
void CC1100_rxFifoHalfFull(void)
{
    dataPending = TRUE;
}


//----------------------------------------------------------------------------------
//  void rxPacketRecvd(void)
//
//  DESCRIPTION:
//    This function is called when a complete packet has been received. Set a flag
//    indicating that radio has received a complete packet.
//----------------------------------------------------------------------------------
void CC1100_rxPacketRecvd(void)
{
    packetComplete = TRUE;
}

uint8_t getpacketComplete()
{
	return packetComplete;
}

void resetpacketComplete()
{
	packetComplete = FALSE;
}

uint8_t getdataPending()
{
	return dataPending;
}

void resetdataPending()
{
	dataPending = FALSE;
}


//----------------------------------------------------------------------------------
//  void rxInit(void)
//
//  DESCRIPTION:
//    Set up chip to operate in RX mode
//----------------------------------------------------------------------------------
void CC1100_rxInit(void)
{
    // Set RX FIFO threshold
    CC1100_RfWriteReg(CC1100_FIFOTHR, FIFO_THRESHOLD);
	//CC1100_RfWriteReg(CC1100_FIFOTHR, 0);
    // Set GDO0 to be RX FIFO threshold signal
    CC1100_RfWriteReg(CC1100_IOCFG0, 0x00);
    // Set GDO2 to be CRC Ok
    CC1100_RfWriteReg(CC1100_IOCFG2, 0x07);
}




void CC1100_rxStart(void)
{
	// Reset state and set radio in RX mode
	/* Safe to set states, as radio is IDLE   */
	GPIOIntClear(CC1100_GDO_GPIO_BASE, CC1100_GDO0| CC1100_GDO2);

	packetComplete = FALSE;
	dataPending    = FALSE;

	CC1100_RfStrobe(CC1100_SRX);
}



//----------------------------------------------------------------------------------
//  uint8 rxRecvPacket(uint8* data, uint16* length)
//
//  DESCRIPTION:
//    Receive packet from radio. Waits for either a FIFO half full event or
//    a packet received event and reads data from the RX FIFO accordingly.
//    Returns when a complete packet is received.
//
//  ARGUMENTS:
//    data    - Pointer to where to write the incoming packet payload
//    length  - Pointer to where to write the length of the packet payload
//
//  RETURNS:
//    RX_OK                - packet was received successfully
//    RX_LENGTH_VIOLATION  - length of the packet is illegal
//    RX_CRC_MISMATCH      - claculated CRC does not match packet CRC
//
//----------------------------------------------------------------------------------
uint8_t CC1100_rxRecvPacket(uint8_t* data, uint8_t* length,uint8_t* rssi)
{
    uint8_t done = FALSE;
    uint8_t lengthByteRead = FALSE;


    uint8_t appendStatus[2];
    uint8_t bytesRead = 0;
    uint8_t bytesRemaining = 0;


	GPIOIntClear(CC1100_GDO_GPIO_BASE, CC1100_GDO0| CC1100_GDO2);

	packetComplete = FALSE;
	dataPending    = FALSE;

	LED_OFF(LED_RED|LED_GREEN);

	CC1100_RfStrobe(CC1100_SRX);

    while (!done)
    {

        // An interrupt has occured. Take the appropriate action
        // according to the state of the system.
        if (packetComplete && !lengthByteRead)
        {

            packetComplete = FALSE;

            CC1100_RfReadFifo(length, 1);
            lengthByteRead = TRUE;

            if (*length == 0 || *length > FIFO_SIZE)
            {
                CC1100_RfStrobe(CC1100_SIDLE);
                CC1100_RfStrobe(CC1100_SFRX);

                return(RX_LENGTH_VIOLATION);
            }

            // Get the complete packet from the FIFO
            CC1100_RfReadFifo(data, *length);
            done = TRUE;
        }
        else if (packetComplete && lengthByteRead)
        {
            packetComplete = FALSE;

            CC1100_RfReadFifo(&data[bytesRead], bytesRemaining);
            done = TRUE;
        }
        else if (dataPending && !lengthByteRead)
        {
            dataPending = FALSE;

            CC1100_RfReadFifo(length, 1);
            lengthByteRead = TRUE;

            if (*length == 0)
            {
                CC1100_RfStrobe(CC1100_SIDLE);
                CC1100_RfStrobe(CC1100_SFRX);

                return(RX_LENGTH_VIOLATION);
            }

            // Read remaining bytes in FIFO, but don't empty the
            // FIFO because of RX FIFO behaviour
            CC1100_RfReadFifo(data, FIFO_THRESHOLD_BYTES - 2);
            bytesRead += FIFO_THRESHOLD_BYTES - 2;
            bytesRemaining = *length - bytesRead;
        }
        else if (dataPending && lengthByteRead)
        {
            dataPending = FALSE;

            // Read remaining bytes in FIFO, but don't empty the
            // FIFO because of RX FIFO behaviour
            CC1100_RfReadFifo(&data[bytesRead], FIFO_THRESHOLD_BYTES - 1);
            bytesRead += FIFO_THRESHOLD_BYTES - 1;
            bytesRemaining = *length - bytesRead;
        }

    }

    // Get the appended status bytes [RSSI, LQI]
    CC1100_RfReadFifo(appendStatus, 2);

    data[*length]=appendStatus[0];

    *rssi=appendStatus[0];

    //Check CRC
    if ((appendStatus[1] & CC1100_LQI_CRC_OK_BM) != CC1100_LQI_CRC_OK_BM)
    {
        return(RX_CRC_MISMATCH);
    }

    return(RX_OK);
}



//----------------------------------------------------------------------------------
//  void txFifoHalfFull(void)
//
//  DESCRIPTION:
//    This function is called when the FIFO Threshold signal is deasserted, indicating
//    that the FIFO (in this example) is half full. Update the number of bytes
//    that are in the FIFO.
//----------------------------------------------------------------------------------
void CC1100_txFifoHalfFull(void)
{
    // This is potentially dangerous, as we cannot guarantee that
    // the ISR is invoked immediately after the condition occurs
    // or if someone is trying to write to the FIFO at the same time
    // (possible to get this interrupt if the radio is sending at the
    // same time). Should be handled with care in software.
    nBytesInFifo = FIFO_THRESHOLD_BYTES;
}


//----------------------------------------------------------------------------------
//  void txPacketSent(void)
//
//  DESCRIPTION:
//    This function is called every time the radio indicates that a packet has
//    been transmitted. Get the number of bytes in the FIFO and strobe TX if
//    there are packets to be sent.
//----------------------------------------------------------------------------------
void CC1100_txPacketSent(void)
{
    nPacketsPending--;
 //   LED_OFF(LED_BLUE);

    // Get number of bytes in TXFIFO.
    // Note the CC1100/CC2500 errata concerning reading from the TXBYTES
    // register. However, since a packet has been transmitted, the radio
    // is no longer in TX state, since (in this particular application),
    // MCSM1.TXOFF_MODE is set to IDLE.
    nBytesInFifo = CC1100_RfReadStatusReg(CC1100_TXBYTES);

    if (nBytesInFifo & 0x80)
        // Oops! TX underflow. There is something seriously wrong
        // Don't try to do anything more.
        while(TRUE);

    if (nPacketsPending > 0 && nBytesInFifo > 0)
    {
        txStrobeNeeded = FALSE;
        CC1100_RfStrobe(CC1100_STX);
    }
    else
    {
        txStrobeNeeded = TRUE;
    }
}


//----------------------------------------------------------------------------------
//  void txInit(void)
//
//  DESCRIPTION:
//    Set up chip to operate in TX mode
//----------------------------------------------------------------------------------
void CC1100_txInit(void)
{
    // Set TX FIFO threshold
    CC1100_RfWriteReg(CC1100_FIFOTHR, FIFO_THRESHOLD);

    // Set GDO0 to be TX FIFO threshold signal
    CC1100_RfWriteReg(CC1100_IOCFG0, 0x02);

    // Set GDO2 to be packet sent signal
    CC1100_RfWriteReg(CC1100_IOCFG2, 0x06);
}


//----------------------------------------------------------------------------------
//  uint8 txSendPacket(uint8* data, uint8 length)
//
//  DESCRIPTION:
//    Send a packet over the air. Use flow control features of the CC1100/CC2500
//    to regulate the number of bytes that can be wirtten to the FIFO at ony time.
//    Return once the packet has been written to the FIFO (i.e. don't wait for the
//    packet to actually be sent).
//
//  ARGUMENTS
//    data   - Data to send. First byte contains length byte
//    length - Total length of packet to send
//
//  RETURNS:
//    This function always returns 0.
//
//----------------------------------------------------------------------------------
uint8_t CC1100_txSendPacket(uint8_t* data, uint8_t length)
{
    uint16_t bytesRemaining = (uint16_t)length;
    uint16_t bytesWritten   = 0;
    uint16_t bytesToWrite;

    nPacketsPending++;

    nBytesInFifo = CC1100_RfReadStatusReg(CC1100_TXBYTES);
    while (bytesRemaining > 0)
    {

        bytesToWrite = MIN(FIFO_SIZE - nBytesInFifo, bytesRemaining);

        if (bytesToWrite > 0)
        {
        	// Write data fragment to FIFO
            CC1100_RfWriteFifo(&data[bytesWritten], bytesToWrite);
            nBytesInFifo += bytesToWrite;

            //LED_ON(LED_BLUE);

            // We don't want to handle a pending FIFO half full interrupt now, since
            // the ISR will say that the FIFO is half full. However, as we have just
            // written to the FIFO, it might actually contain more bytes.
            GPIOIntClear(CC1100_GDO_GPIO_BASE, CC1100_GDO0);

            bytesWritten   += bytesToWrite;
            bytesRemaining -= bytesToWrite;

            if (txStrobeNeeded)
            {
            	txStrobeNeeded = FALSE;
            	CC1100_RfStrobe(CC1100_STX);
            }
        }
    }

    return nBytesInFifo;
}


