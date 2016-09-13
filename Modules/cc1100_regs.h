/***********************************************************************************
    Filename: cc1100.h

    Copyright 2007 Texas Instruments, Inc.
***********************************************************************************/

#ifndef CC1100_REGS_H
#define CC1100_REGS_H
// Rf settings for CC1101
#define CC1100_IOCFG2         0x0000       // IOCFG2              GDO2 Output Pin Configuration
#define CC1100_IOCFG1         0x0001       // IOCFG1              GDO1 Output Pin Configuration
#define CC1100_IOCFG0         0x0002       // IOCFG0              GDO0 Output Pin Configuration
#define CC1100_FIFOTHR        0x0003       // FIFOTHR             RX FIFO and TX FIFO Thresholds
#define CC1100_SYNC1          0x0004       // SYNC1               Sync Word, High Byte
#define CC1100_SYNC0          0x0005       // SYNC0               Sync Word, Low Byte
#define CC1100_PKTLEN         0x0006       // PKTLEN              Packet Length
#define CC1100_PKTCTRL1       0x0007       // PKTCTRL1            Packet Automation Control
#define CC1100_PKTCTRL0       0x0008       // PKTCTRL0            Packet Automation Control
#define CC1100_ADDR           0x0009       // ADDR                Device Address
#define CC1100_CHANNR         0x000A       // CHANNR              Channel Number
#define CC1100_FSCTRL1        0x000B       // FSCTRL1             Frequency Synthesizer Control
#define CC1100_FSCTRL0        0x000C       // FSCTRL0             Frequency Synthesizer Control
#define CC1100_FREQ2          0x000D       // FREQ2               Frequency Control Word, High Byte
#define CC1100_FREQ1          0x000E       // FREQ1               Frequency Control Word, Middle Byte
#define CC1100_FREQ0          0x000F       // FREQ0               Frequency Control Word, Low Byte
#define CC1100_MDMCFG4        0x0010       // MDMCFG4             Modem Configuration
#define CC1100_MDMCFG3        0x0011       // MDMCFG3             Modem Configuration
#define CC1100_MDMCFG2        0x0012       // MDMCFG2             Modem Configuration
#define CC1100_MDMCFG1        0x0013       // MDMCFG1             Modem Configuration
#define CC1100_MDMCFG0        0x0014       // MDMCFG0             Modem Configuration
#define CC1100_DEVIATN        0x0015       // DEVIATN             Modem Deviation Setting
#define CC1100_MCSM2          0x0016       // MCSM2               Main Radio Control State Machine Configuration
#define CC1100_MCSM1          0x0017       // MCSM1               Main Radio Control State Machine Configuration
#define CC1100_MCSM0          0x0018       // MCSM0               Main Radio Control State Machine Configuration
#define CC1100_FOCCFG         0x0019       // FOCCFG              Frequency Offset Compensation Configuration
#define CC1100_BSCFG          0x001A       // BSCFG               Bit Synchronization Configuration
#define CC1100_AGCCTRL2       0x001B       // AGCCTRL2            AGC Control
#define CC1100_AGCCTRL1       0x001C       // AGCCTRL1            AGC Control
#define CC1100_AGCCTRL0       0x001D       // AGCCTRL0            AGC Control
#define CC1100_WOREVT1        0x001E       // WOREVT1             High Byte Event0 Timeout
#define CC1100_WOREVT0        0x001F       // WOREVT0             Low Byte Event0 Timeout
#define CC1100_WORCTRL        0x0020       // WORCTRL             Wake On Radio Control
#define CC1100_FREND1         0x0021       // FREND1              Front End RX Configuration
#define CC1100_FREND0         0x0022       // FREND0              Front End TX Configuration
#define CC1100_FSCAL3         0x0023       // FSCAL3              Frequency Synthesizer Calibration
#define CC1100_FSCAL2         0x0024       // FSCAL2              Frequency Synthesizer Calibration
#define CC1100_FSCAL1         0x0025       // FSCAL1              Frequency Synthesizer Calibration
#define CC1100_FSCAL0         0x0026       // FSCAL0              Frequency Synthesizer Calibration
#define CC1100_RCCTRL1        0x0027       // RCCTRL1             RC Oscillator Configuration
#define CC1100_RCCTRL0        0x0028       // RCCTRL0             RC Oscillator Configuration
#define CC1100_FSTEST         0x0029       // FSTEST              Frequency Synthesizer Calibration Control
#define CC1100_PTEST          0x002A       // PTEST               Production Test
#define CC1100_AGCTEST        0x002B       // AGCTEST             AGC Test
#define CC1100_TEST2          0x002C       // TEST2               Various Test Settings
#define CC1100_TEST1          0x002D       // TEST1               Various Test Settings
#define CC1100_TEST0          0x002E       // TEST0               Various Test Settings
#define CC1100_PARTNUM        0x0030       // PARTNUM             Chip ID
#define CC1100_VERSION        0x0031       // VERSION             Chip ID
#define CC1100_FREQEST        0x0032       // FREQEST             Frequency Offset Estimate from Demodulator
#define CC1100_LQI            0x0033       // LQI                 Demodulator Estimate for Link Quality
#define CC1100_RSSI           0x0034       // RSSI                Received Signal Strength Indication
#define CC1100_MARCSTATE      0x0035       // MARCSTATE           Main Radio Control State Machine State
#define CC1100_WORTIME1       0x0036       // WORTIME1            High Byte of WOR Time
#define CC1100_WORTIME0       0x0037       // WORTIME0            Low Byte of WOR Time
#define CC1100_PKTSTATUS      0x0038       // PKTSTATUS           Current GDOx Status and Packet Status
#define CC1100_VCO_VC_DAC     0x0039       // VCO_VC_DAC          Current Setting from PLL Calibration Module
#define CC1100_TXBYTES        0x003A       // TXBYTES             Underflow and Number of Bytes
#define CC1100_RXBYTES        0x003B       // RXBYTES             Overflow and Number of Bytes
#define CC1100_RCCTRL1_STATUS 0x003C       // RCCTRL1_STATUS      Last RC Oscillator Calibration Result
#define CC1100_RCCTRL0_STATUS 0x003D       // RCCTRL0_STATUS      Last RC Oscillator Calibration Result


// Multi byte memory locations
#define CC1100_PATABLE          0x3E
#define CC1100_TXFIFO           0x3F
#define CC1100_RXFIFO           0x3F

// Definitions for burst/single access to registers
#define CC1100_WRITE_BURST      0x40
#define CC1100_READ_SINGLE      0x80
#define CC1100_READ_BURST       0xC0

// Strobe commands
#define CC1100_SRES             0x30        // Reset chip.
#define CC1100_SFSTXON          0x31        // Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1).
                                            // If in RX/TX: Go to a wait state where only the synthesizer is
                                            // running (for quick RX / TX turnaround).
#define CC1100_SXOFF            0x32        // Turn off crystal oscillator.
#define CC1100_SCAL             0x33        // Calibrate frequency synthesizer and turn it off
                                            // (enables quick start).
#define CC1100_SRX              0x34        // Enable RX. Perform calibration first if coming from IDLE and
                                            // MCSM0.FS_AUTOCAL=1.
#define CC1100_STX              0x35        // In IDLE state: Enable TX. Perform calibration first if
                                            // MCSM0.FS_AUTOCAL=1. If in RX state and CCA is enabled:
                                            // Only go to TX if channel is clear.
#define CC1100_SIDLE            0x36        // Exit RX / TX, turn off frequency synthesizer and exit
                                            // Wake-On-Radio mode if applicable.
#define CC1100_SAFC             0x37        // Perform AFC adjustment of the frequency synthesizer
#define CC1100_SWOR             0x38        // Start automatic RX polling sequence (Wake-on-Radio)
#define CC1100_SPWD             0x39        // Enter power down mode when CSn goes high.
#define CC1100_SFRX             0x3A        // Flush the RX FIFO buffer.
#define CC1100_SFTX             0x3B        // Flush the TX FIFO buffer.
#define CC1100_SWORRST          0x3C        // Reset real time clock.
#define CC1100_SNOP             0x3D        // No operation. May be used to pad strobe commands to two
                                            // bytes for simpler software.


//----------------------------------------------------------------------------------
// Chip Status Byte
//----------------------------------------------------------------------------------

// Bit fields in the chip status byte
#define CC1100_STATUS_CHIP_RDYn_BM             0x80
#define CC1100_STATUS_STATE_BM                 0x70
#define CC1100_STATUS_FIFO_BYTES_AVAILABLE_BM  0x0F

// Chip states
#define CC1100_STATE_IDLE                      0x00
#define CC1100_STATE_RX                        0x10
#define CC1100_STATE_TX                        0x20
#define CC1100_STATE_FSTXON                    0x30
#define CC1100_STATE_CALIBRATE                 0x40
#define CC1100_STATE_SETTLING                  0x50
#define CC1100_STATE_RX_OVERFLOW               0x60
#define CC1100_STATE_TX_UNDERFLOW              0x70


//----------------------------------------------------------------------------------
// Other register bit fields
//----------------------------------------------------------------------------------
#define CC1100_LQI_CRC_OK_BM                   0x80
#define CC1100_LQI_EST_BM                      0x7F


/**********************************************************************************/
#endif
