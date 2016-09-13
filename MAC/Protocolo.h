/*
 * Protocolo.h
 *
 *  Created on: 02/10/2015
 *      Author: Pedro Henrique Fernandes
 */

#ifndef FREERTOS_REDE_PROTOCOLO_H_
#define FREERTOS_REDE_PROTOCOLO_H_

#include <stdint.h>


//Definição do número de timeslots que é possível permanecer
//na rede sem ser kickado ;)
#define NOFF_MAX		0x10

//Número máximo de nós
#define NMAX 			0x02

//Tempo em milisegundos de cada Timeslot
#define TMS_REDE		200


//Tipos de mensagens recebidos
#define TM_DADOS		0x00
#define TM_TRIGGER 		0x01
#define TM_BEACON  		0x02
#define TM_CONFIRMACAO	0X03
#define TM_NOVOEND		0X04
#define TM_FIMREDE		0X05

#define TIMEOFF			0x10

#define REDE_OK			0x01
#define REDE_INVALIDMODE	0x00

//Modos de operação do rádio
#define REDE_MASTER		0x01
#define REDE_SLAVE		0x02
#define REDE_MODOS		0x03

//Endereços default
#define MASTER_END		0x01
#define SLAVE_SEMREDE	0x02
#define BROADCAST		0x00

#define DELAY_TX_RX		400
#define Radio_			CC1100_


//Eventos da Rede
#define TIME_SLICE 	0x01
#define RECEIVE 	0x02
#define SENT    	0x03



typedef struct {
	uint8_t ID;
	uint16_t TMS_TX;
	uint32_t TREDE;
	uint8_t NEND;
	uint8_t END;
	uint8_t TT;
	uint8_t MODO;
}RedeConfig;


uint32_t MasterTaskInit(void);
void CriaRede();

#endif /* FREERTOS_DEMO_V7_0_REDE_PROTOCOLO_H_ */
