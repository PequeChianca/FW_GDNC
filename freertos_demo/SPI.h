/*
 * SPI.h
 *
 *  Created on: 20/08/2015
 *      Author: User
 */

#ifndef PERIFERICOS_SPI_H_
#define PERIFERICOS_SPI_H_



//*****************************************************************************
//
// Configure SSIx in master TI mode.
//
//*****************************************************************************

void SPI0_Init(uint32_t ui32BitRate, uint32_t ui32DataWidth);
uint8_t SPI0_Read(uint8_t addr, uint8_t* data, uint16_t len);
uint8_t SPI0_Write(uint8_t addr, const uint8_t* data, uint16_t len);
uint8_t SPI0_Strobe(uint8_t cmd);

void SPI1_Init(uint32_t ui32BitRate, uint32_t ui32DataWidth);
uint8_t SPI1_Read(uint8_t addr, uint8_t* data, uint16_t len);
uint8_t SPI1_Write(uint8_t addr, const uint8_t* data, uint16_t len);
uint8_t SPI1_Strobe(uint8_t cmd);


void SPI2_Init(uint32_t ui32BitRate, uint32_t ui32DataWidth);
uint8_t SPI2_Read(uint8_t addr, uint8_t* data, uint16_t len);
uint8_t SPI2_Write(uint8_t addr, const uint8_t* data, uint16_t len);
uint8_t SPI2_Strobe(uint8_t cmd);

void SPI3_Init(uint32_t ui32BitRate, uint32_t ui32DataWidth);
uint32_t SPI3_Read(uint8_t addr, uint8_t* data, uint16_t len);
uint32_t SPI3_Write(uint8_t addr, const uint8_t* data, uint16_t len);
uint32_t SPI3_Strobe(uint8_t cmd);

#endif /* PERIFERICOS_SPI_H_ */
