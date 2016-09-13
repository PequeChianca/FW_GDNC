/*
 * Protocolo.h
 *
 *  Created on: 02/10/2015
 *      Author: Pedro Henrique Fernandes
 */

#ifndef REDE_PROTOCOLO_H_
#define REDE_PROTOCOLO_H_

#include <stdint.h>


//Aqui pode mexer:

//Definição do número de timeslots que é possível permanecer
//na rede sem ser kickado ;)
#define NOFF_MAX		0x10 		//configurável

//Número máximo de nós
#define NMAX 			0x01		//configurável

//Número de vezes que o Mestre envia o pacote de reset
#define N_RESET			0x03		//configurável

//Tempo em milisegundos de cada Timeslot
#define TMS_REDE		100			//configurável

//Daqui pra baixo NÃO MEXEEEEEEEEEEEEEEEER!!!!!!!!!!

//Configurações da rede
//Se mudar o tamanho da fifo o bagulho desgrigola!!!!!!!!
//a fifo é circular só pq o tamanho dela é 8 bits, aí quando estoura o
//contador ele vai pro começo e tals..
#define FIFO_TX_LENGTH 64
#define FIFO_RX_LENGTH 64

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
#define TIME_SLICE   0x01
#define SENT_PKT	 0x02
#define RECEIVED_PKT 0x03
#define HALFFULL_TX	 0x04
#define HALFFULL_RX  0x05
#define MESTRE		 0x06
#define ESCRAVO	     0x07

//Número de bytes máximo para a Mensagem
#define MAX_MSGLEN 64

enum Estados {BEACON, TRIGGER, LISTEN_CFG,LISTEN_PCT,END_NET,NEW_NET,LISTEN_CONF,SENDADDR,INIT,SLEEP,SEARCH_NET,SEND_CONFIRM,WAIT_ADDR,SEND_PCT,LISTEN_NET};

typedef struct {
	uint8_t ID;
	uint16_t TMS_TX;
	uint32_t TREDE;
	uint8_t NEND;
	uint8_t END;
	uint8_t TT;
	uint8_t MODO;
}RedeConfig;

//
typedef struct RedeMessage
 {
    uint8_t ucOrigAddr;
    uint8_t ucDestAddr;
    uint8_t ucLenght;
    uint8_t ucType;
    uint32_t uwTime;
    uint8_t ucData[MAX_MSGLEN];
 } xMessage;

//Master e Slave
void LigaRadio();
//Manda o radio pra RX
void SetRxMode();

uint8_t PackTxFifo();
uint8_t SetModo(uint8_t Modo);


void RodaRede();
//Slave
void SaidaRede();

//Master
void CriaRede();
void ResetaRede();
uint8_t EnviaBeacon();
void Trigger();

//Manuseio dos pacotes recebidos
uint8_t Protocolo_rxRecvPacket();

//Manuseio das fifos
uint8_t Pop_RXFifo(uint8_t *Addr,uint8_t *Data,uint8_t *Length);
uint8_t Send(xMessage msg);
static void MACTask(void *pvParameters);
uint32_t MACTaskInit(void);
void txPacketSent();
void rxPacketRecvd();
void rxFifoHalfFull();
void txFifoHalfFull();
void IntGPIOb(void);
void TimerTimeSliceIntHandler(void);
uint8_t txSendPacket(uint8_t* data, uint8_t length);
#endif /* REDE_PROTOCOLO_H_ */
