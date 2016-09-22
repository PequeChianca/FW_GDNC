/*
 * Protocolo.h
 *
 *  Created on: 02/10/2015
 *      Author: Pedro Henrique Fernandes
 */

#ifndef FREERTOS_REDE_RADIO_H_
#define FREERTOS_REDE_RADIO_H_

#include <stdint.h>

//Configurações da rede
//Se mudar o tamanho da fifo o bagulho desgrigola!!!!!!!!
//a fifo é circular só pq o tamanho dela é 8 bits, aí quando estoura o
//contador ele vai pro começo e tals..
#define FIFO_TX_LENGTH 256
#define FIFO_RX_LENGTH 256

#define RADIO_OK			0x01
#define RADIO_INVALIDMODE	0x00

#define CFG_NO_BROADCAST	0x05
#define CFG_BROADCAST		0x07

#define INIT_RADIO	 0x01
#define LISTEN		 0x02
#define SEND		 0x03
#define SENT_PKT	 0x04
#define RECEIVED_PKT 0x05
#define HALFFULL_TX	 0x06
#define HALFFULL_RX  0x07
#define SLEEP		 0x08

extern uint32_t RadioTaskInit(void);

void LigaRadio();
uint8_t Pop_RXFifo(uint8_t *Addr,uint8_t *Data,uint8_t *Length);
void SetRxMode();

void rxPacketRecvd();
void rxFifoHalfFull();
void txFifoHalfFull();
void txPacketSent();
uint8_t	Push_TXFifo(uint8_t Addr,uint8_t END,uint8_t TipoMsg,uint8_t *Data,uint8_t Length);
uint8_t Pop_TXFifo();

#endif
