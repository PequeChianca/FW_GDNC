/*
 * CC1100.h
 *
 *  Created on: 19/08/2015
 *      Author: Pedro Henrique Fernandes
 */

#ifndef _CC1100_H_
#define _CC1100_H_

#include <stdint.h>
//----------------------------------------------------------------------------------
// Type declarations
//----------------------------------------------------------------------------------

#define CC1100_GDO_GPIO_PERIPH     SYSCTL_PERIPH_GPIOB
#define CC1100_GDO_GPIO_BASE       GPIO_PORTB_BASE

#define CC1100_GDO0             GPIO_PIN_5
#define CC1100_GDO2             GPIO_PIN_4

#define CC1100_INT_BASE         INT_GPIOB

// The following structure can be used when configuring the RF chip. SmartRF Studio
// can be used to export a predefined instance of this structure.
typedef struct {
	uint8_t iocfg0;         //GDO0 Output Pin Configuration
	uint8_t fifothr;        //RX FIFO and TX FIFO Thresholds
	uint8_t pktctrl0;       //Packet Automation Control
	uint8_t fsctrl1;        //Frequency Synthesizer Control
	uint8_t freq2;          //Frequency Control Word, High Byte
	uint8_t freq1;          //Frequency Control Word, Middle Byte
	uint8_t freq0;          //Frequency Control Word, Low Byte
	uint8_t mdmcfg4;        //Modem Configuration
	uint8_t mdmcfg3;        //Modem Configuration
	uint8_t mdmcfg2;        //Modem Configuration
	uint8_t deviatn;        //Modem Deviation Setting
	uint8_t mcsm0;          //Main Radio Control State Machine Configuration
	uint8_t foccfg;         //Frequency Offset Compensation Configuration
	uint8_t worctrl;        //Wake On Radio Control
	uint8_t fscal3;         //Frequency Synthesizer Calibration
	uint8_t fscal2;         //Frequency Synthesizer Calibration
	uint8_t fscal1;         //Frequency Synthesizer Calibration
	uint8_t fscal0;         //Frequency Synthesizer Calibration
	uint8_t test2;          //Various Test Settings
	uint8_t test1;          //Various Test Settings
	uint8_t test0;          //Various Test Settings
} HAL_RF_CONFIG;



// For rapid chip configuration with a minimum of function overhead.
// The array has to be set up with predefined values for all registers.
typedef uint8_t HAL_RF_BURST_CONFIG[47];

// The chip status byte, returned by chip for all SPI accesses
typedef uint8_t HAL_RF_STATUS;

//----------------------------------------------------------------------------------
//  Constants used in this file
//----------------------------------------------------------------------------------
#define RX_OK                0
#define RX_LENGTH_VIOLATION  1
#define RX_CRC_MISMATCH      2
#define RX_FIFO_OVERFLOW     3
#define RX_TIMEOUT           4

// In this example, the threshold value is set such that an interrupt is generated
// when the FIFO is half full. It would be possible to set the threshold value
// differently - and thus get an interrupt earlier (meaning there are a few bytes
// available in the FIFO, if interrupt on rising edge) or later (meaning that
// the FIFO is almost full, if interrupt on rising edge).
#define FIFO_THRESHOLD        0x07
#define FIFO_THRESHOLD_BYTES  32
#define FIFO_SIZE             64

//----------------------------------------------------------------------------------
//  Variables used in this file
//----------------------------------------------------------------------------------
static volatile uint8_t dataPending;
static uint8_t packetComplete;

void CC1100_RfPowerOn();
void  CC1100_RfConfig(const HAL_RF_CONFIG* rfConfig, const uint8_t* rfPaTable, uint8_t rfPaTableLen);
void  CC1100_RfBurstConfig(const HAL_RF_BURST_CONFIG rfConfig, const uint8_t* rfPaTable, uint8_t rfPaTableLen);
void  CC1100_RfResetChip(void);

uint8_t CC1100_RfGetChipId(void);
uint8_t CC1100_RfGetChipVer(void);
uint8_t CC1100_RfReadStatusReg(uint8_t addr);
uint8_t CC1100_RfReadReg(uint8_t addr);

HAL_RF_STATUS CC1100_RfWriteReg(uint8_t addr, uint8_t data);
HAL_RF_STATUS CC1100_RfWriteFifo(const uint8_t* data, uint8_t length);
HAL_RF_STATUS CC1100_RfReadFifo(uint8_t* data, uint8_t length);
HAL_RF_STATUS CC1100_RfStrobe(uint8_t cmd);
HAL_RF_STATUS CC1100_RfGetTxStatus(void);
HAL_RF_STATUS CC1100_RfGetRxStatus(void);

void CC1100_rxFifoHalfFull(void);
void CC1100_rxPacketRecvd(void);
void CC1100_rxInit(void);
void CC1100_rxSet(void);
void CC1100_rxStart(void);
uint8_t CC1100_rxRecvPacket(uint8_t* data, uint8_t* length,uint8_t* rssi);
void CC1100_txFifoHalfFull(void);
void CC1100_txPacketSent(void);
void CC1100_txInit(void);

uint8_t getpacketComplete();
void resetpacketComplete();

uint8_t getdataPending();
void resetdataPending();

uint8_t CC1100_txSendPacket(uint8_t* data, uint8_t length);
#endif
