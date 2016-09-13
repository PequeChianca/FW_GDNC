/*
 * CC2500.c
 *
 *  Created on: 19/08/2015
 *      Author: User
 */

#include "cc2500_regs.h"
#include "CC2500.h"

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "../tivac.h"
#include "../SPI.h"



void CC2500_RfResetChip(void)
{
	uint32_t pui32DataRx;

    // Toggle chip select signal
	SSIDataPut(SSI0_BASE,0);
    WaitUs(30);
    SSIDataPut(SSI0_BASE,0);
    WaitUs(45);

    // Send SRES command

    while(SSIBusy(SSI0_BASE))
    {
    }

    SSIDataPut(SSI0_BASE,CC2500_SRES);

    while(SSIBusy(SSI0_BASE))
    {
    }

    // Wait for chip to finish internal reset
    while(SSIDataGetNonBlocking(SSI0_BASE, &pui32DataRx))
	{
	}
}



//----------------------------------------------------------------------------------
//  void halRfConfig(const HAL_RF_CONFIG* rfConfig, const uint8* rfPaTable, uint8 rfPaTableLen)
//
//  DESCRIPTION:
//    Used to configure the CC1100/CC2500 registers with exported register
//    settings from SmartRF Studio.
//
//  ARGUMENTS:
//    rfConfig     - register settings (as exported from SmartRF Studio)
//    rfPaTable    - array of PA table values (from SmartRF Studio)
//    rfPaTableLen - length of PA table
//
//----------------------------------------------------------------------------------
void CC2500_RfConfig(const HAL_RF_CONFIG* rfConfig, const uint8_t* rfPaTable, uint8_t rfPaTableLen)
{
	CC2500_RfWriteReg(CC2500_FSCTRL1,  rfConfig->fsctrl1);    // Frequency synthesizer control.
    CC2500_RfWriteReg(CC2500_FSCTRL0,  rfConfig->fsctrl0);    // Frequency synthesizer control.
    CC2500_RfWriteReg(CC2500_FREQ2,    rfConfig->freq2);      // Frequency control word, high byte.
    CC2500_RfWriteReg(CC2500_FREQ1,    rfConfig->freq1);      // Frequency control word, middle byte.
    CC2500_RfWriteReg(CC2500_FREQ0,    rfConfig->freq0);      // Frequency control word, low byte.
    CC2500_RfWriteReg(CC2500_MDMCFG4,  rfConfig->mdmcfg4);    // Modem configuration.
    CC2500_RfWriteReg(CC2500_MDMCFG3,  rfConfig->mdmcfg3);    // Modem configuration.
    CC2500_RfWriteReg(CC2500_MDMCFG2,  rfConfig->mdmcfg2);    // Modem configuration.
    CC2500_RfWriteReg(CC2500_MDMCFG1,  rfConfig->mdmcfg1);    // Modem configuration.
    CC2500_RfWriteReg(CC2500_MDMCFG0,  rfConfig->mdmcfg0);    // Modem configuration.
    CC2500_RfWriteReg(CC2500_CHANNR,   rfConfig->channr);     // Channel number.
    CC2500_RfWriteReg(CC2500_DEVIATN,  rfConfig->deviatn);    // Modem deviation setting (when FSK modulation is enabled).
    CC2500_RfWriteReg(CC2500_FREND1,   rfConfig->frend1);     // Front end RX configuration.
    CC2500_RfWriteReg(CC2500_FREND0,   rfConfig->frend0);     // Front end RX configuration.
    CC2500_RfWriteReg(CC2500_MCSM0,    rfConfig->mcsm0);      // Main Radio Control State Machine configuration.
    CC2500_RfWriteReg(CC2500_FOCCFG,   rfConfig->foccfg);     // Frequency Offset Compensation Configuration.
    CC2500_RfWriteReg(CC2500_BSCFG,    rfConfig->bscfg);      // Bit synchronization Configuration.
    CC2500_RfWriteReg(CC2500_AGCCTRL2, rfConfig->agcctrl2);   // AGC control.
    CC2500_RfWriteReg(CC2500_AGCCTRL1, rfConfig->agcctrl1);   // AGC control.
    CC2500_RfWriteReg(CC2500_AGCCTRL0, rfConfig->agcctrl0);   // AGC control.
    CC2500_RfWriteReg(CC2500_FSCAL3,   rfConfig->fscal3);     // Frequency synthesizer calibration.
    CC2500_RfWriteReg(CC2500_FSCAL2,   rfConfig->fscal2);     // Frequency synthesizer calibration.
    CC2500_RfWriteReg(CC2500_FSCAL1,   rfConfig->fscal1);     // Frequency synthesizer calibration.
    CC2500_RfWriteReg(CC2500_FSCAL0,   rfConfig->fscal0);     // Frequency synthesizer calibration.
    CC2500_RfWriteReg(CC2500_FSTEST,   rfConfig->fstest);     // Frequency synthesizer calibration.
    CC2500_RfWriteReg(CC2500_TEST2,    rfConfig->test2);      // Various test settings.
    CC2500_RfWriteReg(CC2500_TEST1,    rfConfig->test1);      // Various test settings.
    CC2500_RfWriteReg(CC2500_TEST0,    rfConfig->test0);      // Various test settings.
    CC2500_RfWriteReg(CC2500_IOCFG2,   rfConfig->iocfg2);     // GDO2 output pin configuration.
    CC2500_RfWriteReg(CC2500_IOCFG0,   rfConfig->iocfg0);     // GDO0 output pin configuration.
    CC2500_RfWriteReg(CC2500_PKTCTRL1, rfConfig->pktctrl1);   // Packet automation control.
    CC2500_RfWriteReg(CC2500_PKTCTRL0, rfConfig->pktctrl0);   // Packet automation control.
    CC2500_RfWriteReg(CC2500_ADDR,     rfConfig->addr);       // Device address.
    CC2500_RfWriteReg(CC2500_PKTLEN,   rfConfig->pktlen);     // Packet length.

    SPI0_Write(CC2500_PATABLE | CC2500_WRITE_BURST, rfPaTable, rfPaTableLen);
}


//----------------------------------------------------------------------------------
//  void  halRfBurstConfig(const HAL_RF_BURST_CONFIG rfConfig, const uint8* rfPaTable, uint8 rfPaTableLen)
//
//  DESCRIPTION:
//    Used to configure all of the CC1100/CC2500 registers in one burst write.
//
//  ARGUMENTS:
//    rfConfig     - register settings
//    rfPaTable    - array of PA table values (from SmartRF Studio)
//    rfPaTableLen - length of PA table
//
//----------------------------------------------------------------------------------
void  CC2500_RfBurstConfig(const HAL_RF_BURST_CONFIG rfConfig, const uint8_t* rfPaTable, uint8_t rfPaTableLen)
{
	SPI0_Write(CC2500_IOCFG2  | CC2500_WRITE_BURST, rfConfig, sizeof(rfConfig));
	SPI0_Write(CC2500_PATABLE | CC2500_WRITE_BURST, rfPaTable, rfPaTableLen);
}

//----------------------------------------------------------------------------------
//  uint8 halRfGetChipId(void)
//----------------------------------------------------------------------------------
uint8_t CC2500_RfGetChipId(void)
{
    return(CC2500_RfReadStatusReg(CC2500_PARTNUM));
}

//----------------------------------------------------------------------------------
//  uint8 halRfGetChipVer(void)
//----------------------------------------------------------------------------------
uint8_t CC2500_RfGetChipVer(void)
{
    return(CC2500_RfReadStatusReg(CC2500_VERSION));
}

//----------------------------------------------------------------------------------
//  HAL_RF_STATUS halRfStrobe(uint8 cmd)
//----------------------------------------------------------------------------------
HAL_RF_STATUS CC2500_RfStrobe(uint8_t cmd)
{
    return(SPI0_Strobe(cmd));
}

//----------------------------------------------------------------------------------
//  uint8 halRfReadStatusReg(uint8 addr)
//
//  NOTE:
//      When reading a status register over the SPI interface while the register
//      is updated by the radio hardware, there is a small, but finite, probability
//      that the result is corrupt. The CC1100 and CC2500 errata notes explain the
//      problem and propose several workarounds.
//
//----------------------------------------------------------------------------------
uint8_t CC2500_RfReadStatusReg(uint8_t addr)
{
    uint8_t reg;
    SPI0_Read(addr | CC2500_READ_BURST, &reg, 1);
    return(reg);
}

//----------------------------------------------------------------------------------
//  uint8 halRfReadReg(uint8 addr)
//----------------------------------------------------------------------------------
uint8_t CC2500_RfReadReg(uint8_t addr)
{
    uint8_t reg;
    SPI0_Read(addr | CC2500_READ_SINGLE, &reg, 1);
    return(reg);
}

//----------------------------------------------------------------------------------
//  HAL_RF_STATUS halRfWriteReg(uint8 addr, uint8 data)
//----------------------------------------------------------------------------------
HAL_RF_STATUS CC2500_RfWriteReg(uint8_t addr, uint8_t data)
{
    uint8_t rc;
    rc =  SPI0_Write(addr, &data, 1);
    return(rc);
}

//----------------------------------------------------------------------------------
//  HAL_RF_STATUS halRfWriteFifo(uint8* data, uint8 length)
//----------------------------------------------------------------------------------
HAL_RF_STATUS CC2500_RfWriteFifo(const uint8_t* data, uint8_t length)
{
    return(SPI0_Write(CC2500_TXFIFO | CC2500_WRITE_BURST, data, length));
}

//----------------------------------------------------------------------------------
//  HAL_RF_STATUS halRfReadFifo(uint8* data, uint8 length)
//----------------------------------------------------------------------------------
HAL_RF_STATUS CC2500_RfReadFifo(uint8_t* data, uint8_t length)
{
    return(SPI0_Read(CC2500_RXFIFO | CC2500_READ_BURST, data, length));
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
//      CC1100 and CC2500 errata notes explain the problem and propose several
//      workarounds.
//
//----------------------------------------------------------------------------------
HAL_RF_STATUS CC2500_RfGetTxStatus(void)
{
    return(SPI0_Strobe(CC2500_SNOP));
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
//      CC1100 and CC2500 errata notes explain the problem and propose several
//      workarounds.
//
//----------------------------------------------------------------------------------
HAL_RF_STATUS CC2500_RfGetRxStatus(void)
{
    return(SPI0_Strobe(CC2500_SNOP | CC2500_READ_SINGLE));
}
