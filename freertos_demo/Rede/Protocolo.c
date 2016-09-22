/*
 * Protocolo.c
 *
 *  Created on: 02/10/2015
 *      Author: Pedro Henrique Fernandes
 */

#include "Protocolo.h"

#include <stdint.h>
#include <stdbool.h>
#include "../SPI.h"
#include "../tivac.h"
#include "../Modules/CC1100.h"
#include "../Modules/CC1100_regs.h"
#include "../my_rf_settings.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/timer.h"
#include "utils/random.h"
#include "drivers/rgb.h"
#include "../hal_defs.h"
#include "FreeRTOS.h"
#include "task.h"
#include "priorities.h"
#include "semphr.h"
#include "list.h"

#define MACTASKSTACKSIZE        2048         // Stack size in words

TaskHandle_t MACTaskHandle = NULL;

QueueHandle_t RxQueue;
QueueHandle_t TxQueue;

uint8_t data[FIFO_TX_LENGTH];

//Valor do tempo da rede no envio do último Beacon
uint32_t TRede_BEACON = 0;
uint32_t TRede_TRIGGER = 0;

//valor retornado pela função de inicialização do timer para resetar o timer
uint16_t Value_TimerReset = 0;

//Apenas o modo master irá conhecer os endereços
uint8_t Enderecos[NMAX];
uint8_t ContMsg[NMAX];

extern uint8_t nPacketsPending;
extern uint8_t txStrobeNeeded;
extern uint16_t nBytesInFifo;
//----------------------------------------------------------------------------------
//  Constants used in this file
//----------------------------------------------------------------------------------
#define RADIO_MODE_TX        1
#define RADIO_MODE_RX        2

#define RX_OK                0
#define RX_LENGTH_VIOLATION  1
#define RX_CRC_MISMATCH      2
#define RX_FIFO_OVERFLOW     3

//----------------------------------------------------------------------------------
//  Variables used in this file
//----------------------------------------------------------------------------------

static void (*GDO0_int)(void);
static void (*GDO2_int)(void);

//Buffer de dados recebidos
static uint32_t ProximaTransmissao = 0;

//Dados da rede
static RedeConfig MinhaRede = { 0, 0, 0, 0, 0, 0, 0 };
extern TaskHandle_t AppTaskHandle;

enum Estados estado = SLEEP;

extern QueueHandle_t xAppQueue;
xMessage xMsg;

static uint32_t g_pui32Colors[3] = { 0x0000, 0x0000, 0x0000 };

uint8_t sending = 0;
uint32_t MACTaskInit(void) {
	RGBInit(1);
	RGBIntensitySet(0.3f);

	//
	// Turn on the Green LED
	//
	g_pui32Colors[1] = 0xFFFF;
	RGBColorSet(g_pui32Colors);

	RxQueue = xQueueCreate(FIFO_RX_LENGTH, sizeof(struct xMessage *));
	TxQueue = xQueueCreate(FIFO_TX_LENGTH, sizeof(struct xMessage *));

	RGBEnable();
	//
	// Create the Master task.
	//
	if (xTaskCreate(MACTask, (signed portCHAR *)"MAC", MACTASKSTACKSIZE, NULL,
			tskIDLE_PRIORITY + PRIORITY_MAC_TASK, &MACTaskHandle) != pdTRUE) {
		return (1);
	}

	//
	// Success.
	//
	return (0);
}

static void MACTask(void *pvParameters) {

	/* Block for 50ms. */
	const TickType_t xDelay = 10 / portTICK_PERIOD_MS;
	uint8_t modo = 0;
	uint8_t LimitBeacon = 0;
	uint8_t Received = 0;
	uint8_t Beacon;
	uint8_t turno = 0;
	uint8_t silenciado = 0;
	uint8_t Reset = 0;
	xMessage *xpMsg;

	uint32_t ulNotifiedValue;
	BaseType_t xResult;

	while (TRUE) {
		xResult = xTaskNotifyWait(0x00, /* Don't clear any notification bits on entry. */
		0xffffffff, /* Reset the notification value to 0 on exit. */
		&ulNotifiedValue, /* Notified value pass out in
		 ulNotifiedValue. */
		portMAX_DELAY); /* Block indefinitely. */

		if (xResult == pdPASS) {

			switch (ulNotifiedValue) {
			case TIME_SLICE:
				switch (estado) {
				case INIT:
					SetModo(modo);
					break;
				case TRIGGER:

					Trigger();
					turno = 0;
					//Rosa
					g_pui32Colors[0] = 255;
					g_pui32Colors[1] = 0;
					g_pui32Colors[2] = 128;
					RGBColorSet(g_pui32Colors);
					estado = LISTEN_PCT;
					break;
				case BEACON:

					if (MinhaRede.NEND < NMAX) {
						LimitBeacon = EnviaBeacon() * 2;
						Beacon = LimitBeacon;
						estado = LISTEN_CFG;
						//Azul
						g_pui32Colors[0] = 0;
						g_pui32Colors[1] = 0;
						g_pui32Colors[2] = 128;
						RGBColorSet(g_pui32Colors);
					} else
						estado = TRIGGER;
					break;
				case END_NET:
					//Vermelho
					g_pui32Colors[0] = 255;
					g_pui32Colors[1] = 0;
					g_pui32Colors[2] = 0;
					RGBColorSet(g_pui32Colors);
					ResetaRede();

					Reset++;
					if (Reset >= N_RESET)
						estado = NEW_NET;

					break;
				case NEW_NET:
					CriaRede();

					Beacon = 0;
					TRede_TRIGGER = 0;
					TRede_BEACON = 0;
					estado = BEACON;
					break;
				case LISTEN_PCT:
					//Amarelo
					g_pui32Colors[0] = 255;
					g_pui32Colors[1] = 255;
					g_pui32Colors[2] = 0;
					RGBColorSet(g_pui32Colors);

					if (turno > 0) {
						if (ContMsg[turno - 1] > NOFF_MAX)
							estado = END_NET;

						if (Received)
							ContMsg[turno - 1] = 0;
						else
							ContMsg[turno - 1]++;
					}

					Received = 0;
					turno++;

					if ((turno > MinhaRede.NEND) && (estado != END_NET)) {
						if (MinhaRede.NEND < NMAX)
							estado = BEACON;
						else
							estado = TRIGGER;
					}
					break;
				case LISTEN_CFG:
					//Laranja
					g_pui32Colors[0] = 255;
					g_pui32Colors[1] = 128;
					g_pui32Colors[2] = 0;
					RGBColorSet(g_pui32Colors);

					if (Beacon > 0)
						Beacon--;
					else
						estado = TRIGGER;
					break;
				case SLEEP:
					break;
				case SENDADDR:
					//verde
					g_pui32Colors[0] = 128;
					g_pui32Colors[1] = 255;
					g_pui32Colors[2] = 0;
					RGBColorSet(g_pui32Colors);
					//Constrói mensagem de novo endereço de rede
					//3 bytes
					xMsg.ucLenght = 3;
					xMsg.ucDestAddr = SLAVE_SEMREDE;
					xMsg.ucType = TM_NOVOEND;
					xMsg.ucData[0] = Enderecos[MinhaRede.NEND];
					xMsg.ucData[1] = MinhaRede.NEND + 1;
					xMsg.ucData[2] = LimitBeacon - Beacon;
					Send(xMsg);
					Beacon--;
					estado = LISTEN_CFG;
					MinhaRede.NEND++;
					break;
				case SEARCH_NET:				//Escravo buscando rede
					//Vermelho
					g_pui32Colors[0] = 255;
					g_pui32Colors[1] = 0;
					g_pui32Colors[2] = 0;
					RGBColorSet(g_pui32Colors);

					break;
				case SEND_CONFIRM://Escravo envia confirmação de que quer entrar na rede
					//Laranja
					g_pui32Colors[0] = 255;
					g_pui32Colors[1] = 128;
					g_pui32Colors[2] = 0;
					RGBColorSet(g_pui32Colors);

					if (MinhaRede.TREDE > ProximaTransmissao)
						estado = SEARCH_NET;

					if (MinhaRede.TREDE == ProximaTransmissao) {
						xMsg.ucLenght = 1;
						xMsg.ucOrigAddr = SLAVE_SEMREDE;
						xMsg.ucDestAddr = MASTER_END;
						xMsg.ucType = TM_CONFIRMACAO;
						xMsg.ucData[0] = ProximaTransmissao;
						Send(xMsg);

						estado = WAIT_ADDR;

						//verde
						g_pui32Colors[0] = 128;
						g_pui32Colors[1] = 255;
						g_pui32Colors[2] = 0;
						RGBColorSet(g_pui32Colors);
					}

					break;
				case WAIT_ADDR:			//Escravo espera receber a confirmação

					//Se bateu o timer e não mudou de estado, significa que o processo de
					//insersão não deu certo, então volta a procurar a rede!!!
					estado = SEARCH_NET;

					break;
				case LISTEN_NET:		//Escravo na Rede, ouvindo seus pacotes
					//Rosa
					g_pui32Colors[0] = 255;
					g_pui32Colors[1] = 0;
					g_pui32Colors[2] = 128;
					RGBColorSet(g_pui32Colors);

					if (MinhaRede.TREDE + 1 == ProximaTransmissao)
						estado = SEND_PCT;

					break;
				case SEND_PCT:
					//Escravo envia pacotes
					//Amarelo
					g_pui32Colors[0] = 255;
					g_pui32Colors[1] = 255;
					g_pui32Colors[2] = 0;
					RGBColorSet(g_pui32Colors);

					//
					if (xQueueReceive(TxQueue, (void * ) &xpMsg,(TickType_t ) 0))
					{
						xpMsg->ucOrigAddr = MinhaRede.END;
						xpMsg->ucType = TM_DADOS;
						Send(*xpMsg);

						//zera quando envia pacotes
						silenciado = 0;
					} else {
						//conta quantos turnos ficou em silencio
						silenciado++;
						//Não deixa a rede morrer enviando um TM_STILLALIVE
						if (silenciado >= (NOFF_MAX + 1)) {
							xMsg.ucLenght = 1;
							xMsg.ucOrigAddr = MinhaRede.END;
							xMsg.ucDestAddr = BROADCAST;
							xMsg.ucType = TM_STILLALIVE;
							xMsg.ucData[0] = MinhaRede.END;
							Send(xMsg);
							turno = 0;
						}
					}
					estado = LISTEN_NET;
					break;
				}
				break;
			case SENT_PKT:
				CC1100_txPacketSent();
				vTaskDelay(xDelay);
				SetRxMode();
				break;
			case RECEIVED_PKT:

				Protocolo_rxRecvPacket();

				if (xQueueReceive(RxQueue, (void * ) &xpMsg, (TickType_t ) 0))
				{
					switch (xpMsg->ucType)
					{
					case TM_DADOS:
						if (xpMsg->ucOrigAddr == Enderecos[turno - 1]) {
							Received = 1;
							ContMsg[turno - 1] = 0;
							xQueueSend(xAppQueue, (void * ) &xpMsg,(TickType_t ) 0);
						} else
						{
							ContMsg[turno - 1]++;
						}
						break;
					case TM_STILLALIVE:

						if (xpMsg->ucOrigAddr == Enderecos[turno - 1]) {
							Received = 1;
							ContMsg[turno - 1] = 0;
						}

						break;
					case TM_TRIGGER:
						//TODO: Slave feature
						if (xpMsg->ucData[0] == MinhaRede.ID) {
							MinhaRede.TREDE = (xpMsg->ucData[1] << 24)
									+ (xpMsg->ucData[2] << 16)
									+ (xpMsg->ucData[3] << 8)
									+ xpMsg->ucData[4];

							if (MinhaRede.TT)
								estado = SEND_PCT;
							else
								ProximaTransmissao = MinhaRede.TREDE
										+ MinhaRede.TT;
						} else {
							SaidaRede();
						}
						break;
					case TM_BEACON:

						break;
					case TM_CONFIRMACAO:

						break;
					case TM_NOVOEND:

						//Checa se o valor recebido é o mesmo que o enviado
						if (xpMsg->ucData[2] == ProximaTransmissao) {
							MinhaRede.END = xpMsg->ucData[0];
							MinhaRede.TT = xpMsg->ucData[1];
							//Muda o filtro de endereço para broadcast
							//Muda o endereço do rádio
							CC1100_RfWriteReg(CC1100_PKTCTRL1, 0x07); // Enable append mode and Addr Check; 0 and 255 -BroadCast
							CC1100_RfWriteReg(CC1100_ADDR, MinhaRede.END);
							estado = LISTEN_NET;
						}

						break;
					case TM_FIMREDE:
						//sai da rede
						SaidaRede();
						break;

					}
				}
				break;
			case HALFFULL_TX:
				break;
			case HALFFULL_RX:
				break;
			case MESTRE:
				modo = REDE_MASTER;
				LigaRadio();
				estado = INIT;
				CriaRede();
				break;
			case ESCRAVO:
				modo = REDE_SLAVE;
				LigaRadio();
				estado = INIT;
				SaidaRede();
				break;
			}
		}
	}

}

//Inicializa o radio CC1100
//Parametrizar as funções para utilizar o mesmo arquivo para os dois rádios
void LigaRadio() {

	//Inicializa o random
	RandomAddEntropy(GetSeed());

	// PowerOn
	CC1100_RfPowerOn();

	// Put radio to sleep
	CC1100_RfStrobe(CC1100_SPWD);

	// Setup chip with register settings from SmartRF Studio
	CC1100_RfConfig(&myRfConfig, PA_TABLE, 8);

	// Additional chip configuration for this example
	CC1100_RfWriteReg(CC1100_MCSM0, 0x18);   // Calibration from IDLE to TX/RX
	CC1100_RfWriteReg(CC1100_MCSM1, 0x0C); // No CCA, RX after RX, IDLE after TX
	CC1100_RfWriteReg(CC1100_PKTCTRL0, 0x45);   // Enable data whitening and CRC
	CC1100_RfWriteReg(CC1100_PKTCTRL1, 0x07);

	SaidaRede();
}

//Poe o radio em modo RX filtrando o endereço
//Nesse modo as únicas msgs que o radio deve ouvir
//são o BEACON, CONFIRMACAO E NOVO ENDEREÇO!!!!!!!!
void SaidaRede() {
	MinhaRede.END = SLAVE_SEMREDE;
	MinhaRede.ID = 0;
	MinhaRede.TREDE = 0;
	MinhaRede.TT = 0;
	MinhaRede.TMS_TX = 0;
	MinhaRede.NEND = 0;

	CC1100_RfWriteReg(CC1100_PKTCTRL1, 0x05);
	CC1100_RfWriteReg(CC1100_ADDR, 0x02);

	//Start the radio in RX mode
	GDO0_int = &rxFifoHalfFull;
	//Trata pacote recebido de acordo com o protocolo
	GDO2_int = &rxPacketRecvd;

	CC1100_rxInit();

	Init_GPIOInt(CC1100_GDO_GPIO_PERIPH,
	CC1100_GDO_GPIO_BASE,
	CC1100_GDO0,
	CC1100_INT_BASE,
	GPIO_FALLING_EDGE,
	configMAX_SYSCALL_INTERRUPT_PRIORITY, &IntGPIOb);

	Init_GPIOInt(CC1100_GDO_GPIO_PERIPH,
	CC1100_GDO_GPIO_BASE,
	CC1100_GDO2,
	CC1100_INT_BASE,
	GPIO_RISING_EDGE,
	configMAX_SYSCALL_INTERRUPT_PRIORITY, &IntGPIOb);

	GPIOIntClear(CC1100_GDO_GPIO_BASE, CC1100_GDO0 | CC1100_GDO2);

	resetpacketComplete();
	resetdataPending();

	CC1100_RfStrobe(CC1100_SRX);
	estado = SEARCH_NET;

}

//Configura o rádio em modo Master ou Slave e retorna se a operaçao deu certo
uint8_t SetModo(uint8_t modo) {

	//Verifica se é uma configuração válida
	if (modo & REDE_MODOS) {
		if (modo == REDE_MASTER) {
			MinhaRede.END = MASTER_END;

			CC1100_RfWriteReg(CC1100_PKTCTRL1, 0x07);
			// Enable append mode and Addr Check; 0 - BroadCast
			CC1100_RfWriteReg(CC1100_ADDR, 0x01);

			CriaRede();
			estado = END_NET;
		} else if (modo == REDE_SLAVE) {
			MinhaRede.END = SLAVE_SEMREDE;
			SaidaRede();
			estado = SEARCH_NET;
		}

		return REDE_OK;

	} else
		return REDE_INVALIDMODE;
}

void SetRxMode() {
	//Set Rx mode
	GPIOIntDisable(CC1100_GDO_GPIO_BASE, CC1100_GDO0 | CC1100_GDO2);
	//Start the radio in RX mode
	GDO0_int = &rxFifoHalfFull;
	//Trata pacote recebido de acordo com o protocolo
	GDO2_int = &rxPacketRecvd;

	CC1100_rxInit();

	Init_GPIOInt(CC1100_GDO_GPIO_PERIPH,
	CC1100_GDO_GPIO_BASE,
	CC1100_GDO0,
	CC1100_INT_BASE,
	GPIO_FALLING_EDGE,
	configMAX_SYSCALL_INTERRUPT_PRIORITY, &IntGPIOb);

	Init_GPIOInt(CC1100_GDO_GPIO_PERIPH,
	CC1100_GDO_GPIO_BASE,
	CC1100_GDO2,
	CC1100_INT_BASE,
	GPIO_RISING_EDGE,
	configMAX_SYSCALL_INTERRUPT_PRIORITY, &IntGPIOb);

	GPIOIntClear(CC1100_GDO_GPIO_BASE, CC1100_GDO0 | CC1100_GDO2);

	CC1100_RfStrobe(CC1100_SRX);

}

//Manuseio dos pacotes recebidos
uint8_t Protocolo_rxRecvPacket() {
	uint8_t data[255];
	uint8_t length;
	uint8_t done = FALSE;
	uint8_t lengthByteRead = FALSE;
	uint8_t appendStatus[2];
	uint8_t bytesRead = 0;
	uint8_t bytesRemaining = 0;
	xMessage *xpMsg;

	resetpacketComplete();
	resetdataPending();

	while (!done) {

		// An interrupt has occured. Take the appropriate action
		// according to the state of the system.
		if (!lengthByteRead) {

			resetpacketComplete();

			CC1100_RfReadFifo(&length, 1);
			lengthByteRead = TRUE;

			if (length == 0 || length > FIFO_SIZE) {
				CC1100_RfStrobe(CC1100_SIDLE);
				CC1100_RfStrobe(CC1100_SFRX);

				return (RX_LENGTH_VIOLATION);
			}

			// Get the complete packet from the FIFO
			CC1100_RfReadFifo(data, length);
			done = TRUE;
		} else if (lengthByteRead) {
			resetpacketComplete();

			CC1100_RfReadFifo(&data[bytesRead], bytesRemaining);
			done = TRUE;
		} else if (dataPending && !lengthByteRead) {
			dataPending = FALSE;

			CC1100_RfReadFifo(&length, 1);
			lengthByteRead = TRUE;

			if (length == 0) {
				CC1100_RfStrobe(CC1100_SIDLE);
				CC1100_RfStrobe(CC1100_SFRX);

				return (RX_LENGTH_VIOLATION);
			}

			// Read remaining bytes in FIFO, but don't empty the
			// FIFO because of RX FIFO behaviour
			CC1100_RfReadFifo(data, FIFO_THRESHOLD_BYTES - 2);
			bytesRead += FIFO_THRESHOLD_BYTES - 2;
			bytesRemaining = length - bytesRead;
		} else if (dataPending && lengthByteRead) {
			dataPending = FALSE;

			// Read remaining bytes in FIFO, but don't empty the
			// FIFO because of RX FIFO behaviour
			CC1100_RfReadFifo(&data[bytesRead], FIFO_THRESHOLD_BYTES - 1);
			bytesRead += FIFO_THRESHOLD_BYTES - 1;
			bytesRemaining = length - bytesRead;
		}

	}

	// Get the appended status bytes [RSSI, LQI]
	CC1100_RfReadFifo(appendStatus, 2);

	//data[0]=Endereço de destino
	//data[1]=Endereço de origem
	//data[2]=Tipo de Mensagem

	//Check CRC
	uint8_t i = 0;
	if ((appendStatus[1] & CC1100_LQI_CRC_OK_BM) == CC1100_LQI_CRC_OK_BM) {
		uint8_t k = 1;

		xMsg.ucLenght = length - 3;		//data[0]
		xMsg.ucType = data[2];		//data[2]
		xMsg.ucOrigAddr = data[1];	//data[1]
		xMsg.ucDestAddr = data[0];	//data[0]

		for (k = 0; k < (xMsg.ucLenght); k++) {
			xMsg.ucData[k] = data[3 + k]; //data[2+k]
		}

		//todo:checar isso aqui!!!!
		xMsg.ucData[xMsg.ucLenght] = 0;
		xMsg.uwTime = MinhaRede.TREDE;
		xpMsg = &xMsg;
		xQueueSend(RxQueue, (void * ) &xpMsg, (TickType_t ) 0);

		//Trata Msg de acordo com o tipo de msg
		switch (xMsg.ucType) {
		case TM_TRIGGER:
			//Pacote de disparo trás o ID da rede e o tempo da rede
			//Caso o Id da rede for desconhecido o rádio deve sair da rede antiga e
			//Ouvir outro Beacon pra entrar na nova rede
			RESETTIME(Value_TimerReset);
			break;
		case TM_BEACON:
			//ID da rede
			MinhaRede.ID = data[3];
			//tempo em milisegundos que a rede está operando
			MinhaRede.TMS_TX = (data[5] << 8) + data[6];
			//Pega o valor pra resetar o timer com o tempo certo
			Value_TimerReset = InitTimer4_TimeSlice(MinhaRede.TMS_TX,
					&TimerTimeSliceIntHandler,
					configMAX_SYSCALL_INTERRUPT_PRIORITY);
			MinhaRede.TREDE = 0;
			ProximaTransmissao = (RandomSeed() % data[4]) * 2 + 1;
			estado = SEND_CONFIRM;

			break;
		case TM_CONFIRMACAO:

			//A confirmação deve vir de um slave fora da rede e a transmissão deve casar com
			//o timeslot do mestre
			//se ainda houver espaço na rede
			if ((data[3] == (MinhaRede.TREDE - TRede_BEACON))
					&& (data[1] == SLAVE_SEMREDE) && (MinhaRede.NEND <= NMAX)) {
				//caso a confirmação seja aceita um novo endereço será gerado
				done = FALSE;

				while (!done) {
					//Endereco pode variar entre 0x03 a 254
					//Pega um valor aleatório entre 0-251 e soma 3
					Enderecos[MinhaRede.NEND] = (RandomSeed() % 252) + 3;

					//Mas não pode haver nenhum endereço igual na rede
					for (i = 0; i < MinhaRede.NEND; i++) {
						//Encerra o for caso exista um endereco igual na rede
						if ((Enderecos[i] == Enderecos[MinhaRede.NEND])
								&& (Enderecos[i] != REDE_SLAVE)) {
							i = MinhaRede.NEND + 1;
							RandomAddEntropy(Enderecos[MinhaRede.NEND]);
						}
					}

					//se o laco foi terminado corretamente i==MinhaRede.NEND
					//e portanto não existem enderecos iguais ao novo na rede.
					if (i == MinhaRede.NEND)
						done = TRUE;
				}

				estado = SENDADDR;
			}
			break;
		}

	}

	//
	return RX_OK;
}

void CriaRede() {
	int i = 0;

	for (i = 0; i < NMAX; i++) {
		ContMsg[i] = 0;
	}

	//Gera um valor não nulo de 8 bits
	MinhaRede.ID = (RandomSeed() % 255) + 1;
	RandomAddEntropy(MinhaRede.ID);
	MinhaRede.NEND = 0;
	MinhaRede.TREDE = 0;
	MinhaRede.TMS_TX = TMS_REDE;
	MinhaRede.TT = 0;

	Value_TimerReset = InitTimer4_TimeSlice(MinhaRede.TMS_TX,
			&TimerTimeSliceIntHandler, configMAX_SYSCALL_INTERRUPT_PRIORITY);
}

void ResetaRede() {
	xMessage msg;

	msg.ucLenght = 1;
	msg.ucOrigAddr = MinhaRede.END;
	msg.ucDestAddr = BROADCAST;
	msg.ucType = TM_FIMREDE;
	msg.ucData[0] = MinhaRede.ID;

	Send(msg);
}

uint8_t EnviaBeacon() {
	uint8_t NID = (NMAX - MinhaRede.NEND) * (NMAX - MinhaRede.NEND);
	xMessage msg;

	msg.ucLenght = 4;
	msg.ucOrigAddr = MinhaRede.END;
	msg.ucDestAddr = SLAVE_SEMREDE;
	msg.ucType = TM_BEACON;
	msg.ucData[0] = MinhaRede.ID;
	msg.ucData[1] = NID;
	msg.ucData[2] = (MinhaRede.TMS_TX >> 8) & 0xff;
	msg.ucData[3] = (MinhaRede.TMS_TX & 0xff);

	Send(msg);
	TRede_BEACON = MinhaRede.TREDE;

	return NID;
}

void Trigger() {
	xMessage msg;

	msg.ucLenght = 5;
	msg.ucOrigAddr = MinhaRede.END;
	msg.ucDestAddr = BROADCAST;
	msg.ucType = TM_TRIGGER;
	msg.ucData[0] = MinhaRede.ID;
	ProximaTransmissao = MinhaRede.TREDE;
	msg.ucData[1] = (ProximaTransmissao >> 24) & 0xff;
	msg.ucData[2] = (ProximaTransmissao >> 16) & 0xff;
	msg.ucData[3] = (ProximaTransmissao >> 8) & 0xff;
	msg.ucData[4] = ProximaTransmissao & 0xff;

	Send(msg);

	TRede_TRIGGER = ProximaTransmissao;
}

uint8_t Send(xMessage msg) {
	uint8_t Data[255];
	uint8_t Length = msg.ucLenght + 4;
	uint8_t Bytes = 0;

	/* Block for 5ms. */
	const TickType_t xDelay = 5 / portTICK_PERIOD_MS;

	GPIOIntDisable(CC1100_GDO_GPIO_BASE, CC1100_GDO0 | CC1100_GDO2);

	// Set up interrupt on GDO0
	GDO0_int = &txFifoHalfFull;
	// Set up interrupt on GDO2
	GDO2_int = &txPacketSent;

	CC1100_txInit();

	//Init GDO0 and GDO2 interrupts
	Init_GPIOInt(CC1100_GDO_GPIO_PERIPH,
	CC1100_GDO_GPIO_BASE,
	CC1100_GDO0 | CC1100_GDO2,
	CC1100_INT_BASE,
	GPIO_FALLING_EDGE,
	configMAX_SYSCALL_INTERRUPT_PRIORITY, &IntGPIOb);

	//TODO: Parametrizar o delay?!
	//Delay para colocar o pacote mais pro "meio" do timeslice
	Data[0] = msg.ucLenght + 3;
	Data[1] = msg.ucDestAddr;
	Data[2] = msg.ucOrigAddr;
	Data[3] = msg.ucType;

	for (Bytes = 0; Bytes < msg.ucLenght; Bytes++) {
		Data[Bytes + 4] = msg.ucData[Bytes];
	}

	vTaskDelay(xDelay);
	Bytes = txSendPacket(Data, Length);
	vTaskDelay(xDelay);

	return Bytes;

}

uint8_t txSendPacket(uint8_t* data, uint8_t length) {
	uint16_t bytesRemaining = (uint16_t) length;
	uint16_t bytesWritten = 0;
	uint16_t bytesToWrite;
	nPacketsPending++;

	//nBytesInFifo = CC1100_RfReadStatusReg(CC1100_TXBYTES);
	SPI3_Read(CC1100_TXBYTES | CC1100_READ_BURST, &nBytesInFifo, 1);
	while (bytesRemaining > 0) {

		bytesToWrite = MIN(FIFO_SIZE - nBytesInFifo, bytesRemaining);

		if (bytesToWrite > 0) {
			// Write data fragment to FIFO
			CC1100_RfWriteFifo(&data[bytesWritten], bytesToWrite);
			nBytesInFifo += bytesToWrite;

			// We don't want to handle a pending FIFO half full interrupt now, since
			// the ISR will say that the FIFO is half full. However, as we have just
			// written to the FIFO, it might actually contain more bytes.
			GPIOIntClear(CC1100_GDO_GPIO_BASE, CC1100_GDO0);

			bytesWritten += bytesToWrite;
			bytesRemaining -= bytesToWrite;

			if (txStrobeNeeded) {
				txStrobeNeeded = FALSE;
				CC1100_RfStrobe(CC1100_STX);
			}
		}
	}

	return nBytesInFifo;
}

void rxPacketRecvd() {
	BaseType_t xHigherPriorityTaskWoken;

	/* xHigherPriorityTaskWoken must be initialised to pdFALSE.  If calling
	 xTaskNotifyFromISR() unblocks the handling task, and the priority of
	 the handling task is higher than the priority of the currently running task,
	 then xHigherPriorityTaskWoken will automatically get set to pdTRUE. */
	xHigherPriorityTaskWoken = pdFALSE;

	/* Unblock the handling task so the task can perform any processing necessitated
	 by the interrupt.  xHandlingTask is the task's handle, which was obtained
	 when the task was created.  The handling task's notification value
	 is bitwise ORed with the interrupt status - ensuring bits that are already
	 set are not overwritten. */
	xTaskNotifyFromISR(MACTaskHandle, RECEIVED_PKT, eSetBits,
			&xHigherPriorityTaskWoken);
	/*
	 Force a context switch if xHigherPriorityTaskWoken is now set to pdTRUE.
	 The macro used to do this is dependent on the port and may be called
	 portEND_SWITCHING_ISR. */
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void rxFifoHalfFull() {
	BaseType_t xHigherPriorityTaskWoken;

	/* xHigherPriorityTaskWoken must be initialised to pdFALSE.  If calling
	 xTaskNotifyFromISR() unblocks the handling task, and the priority of
	 the handling task is higher than the priority of the currently running task,
	 then xHigherPriorityTaskWoken will automatically get set to pdTRUE. */
	xHigherPriorityTaskWoken = pdFALSE;

	/* Unblock the handling task so the task can perform any processing necessitated
	 by the interrupt.  xHandlingTask is the task's handle, which was obtained
	 when the task was created.  The handling task's notification value
	 is bitwise ORed with the interrupt status - ensuring bits that are already
	 set are not overwritten. */
	xTaskNotifyFromISR(MACTaskHandle, HALFFULL_RX, eSetBits,
			&xHigherPriorityTaskWoken);
	/*
	 Force a context switch if xHigherPriorityTaskWoken is now set to pdTRUE.
	 The macro used to do this is dependent on the port and may be called
	 portEND_SWITCHING_ISR. */
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void txPacketSent() {
	BaseType_t xHigherPriorityTaskWoken;

	/* xHigherPriorityTaskWoken must be initialised to pdFALSE.  If calling
	 xTaskNotifyFromISR() unblocks the handling task, and the priority of
	 the handling task is higher than the priority of the currently running task,
	 then xHigherPriorityTaskWoken will automatically get set to pdTRUE. */
	xHigherPriorityTaskWoken = pdFALSE;

	/* Unblock the handling task so the task can perform any processing necessitated
	 by the interrupt.  xHandlingTask is the task's handle, which was obtained
	 when the task was created.  The handling task's notification value
	 is bitwise ORed with the interrupt status - ensuring bits that are already
	 set are not overwritten. */
	xTaskNotifyFromISR(MACTaskHandle, SENT_PKT, eSetBits,
			&xHigherPriorityTaskWoken);
	/*
	 Force a context switch if xHigherPriorityTaskWoken is now set to pdTRUE.
	 The macro used to do this is dependent on the port and may be called
	 portEND_SWITCHING_ISR. */
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void txFifoHalfFull() {
	CC1100_txFifoHalfFull();

	BaseType_t xHigherPriorityTaskWoken;

	/* xHigherPriorityTaskWoken must be initialised to pdFALSE.  If calling
	 xTaskNotifyFromISR() unblocks the handling task, and the priority of
	 the handling task is higher than the priority of the currently running task,
	 then xHigherPriorityTaskWoken will automatically get set to pdTRUE. */
	xHigherPriorityTaskWoken = pdFALSE;

	/* Unblock the handling task so the task can perform any processing necessitated
	 by the interrupt.  xHandlingTask is the task's handle, which was obtained
	 when the task was created.  The handling task's notification value
	 is bitwise ORed with the interrupt status - ensuring bits that are already
	 set are not overwritten. */
	xTaskNotifyFromISR(MACTaskHandle, HALFFULL_TX, eSetBits,
			&xHigherPriorityTaskWoken);
	/*
	 Force a context switch if xHigherPriorityTaskWoken is now set to pdTRUE.
	 The macro used to do this is dependent on the port and may be called
	 portEND_SWITCHING_ISR. */
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}

//----------------------------------------------------------------------------------
//  void txISR(void)
//
//  DESCRIPTION:
//    This function is called (in interrupt context) every time a packet has been
//    transmitted.
//----------------------------------------------------------------------------------
void IntGPIOb(void) {
	volatile uint32_t Reg;

	Reg = GPIOIntStatus(CC1100_GDO_GPIO_BASE, CC1100_GDO0 | CC1100_GDO2);

	switch (Reg) {
	case CC1100_GDO0:
		GDO0_int();
		GPIOIntClear(CC1100_GDO_GPIO_BASE, CC1100_GDO0);
		break;
	case CC1100_GDO2:
		GDO2_int();
		GPIOIntClear(CC1100_GDO_GPIO_BASE, CC1100_GDO2);
		break;
	default:
		break;
	}

}

//Interrupção do timer, definido pelo TMS_REDE em Protocolo.h
//Ou, no caso do slave é definido pelo recebimento do Beacon
void TimerTimeSliceIntHandler(void) {
	BaseType_t xHigherPriorityTaskWoken;

	MinhaRede.TREDE++;

	/* xHigherPriorityTaskWoken must be initialised to pdFALSE.  If calling
	 xTaskNotifyFromISR() unblocks the handling task, and the priority of
	 the handling task is higher than the priority of the currently running task,
	 then xHigherPriorityTaskWoken will automatically get set to pdTRUE. */
	xHigherPriorityTaskWoken = pdFALSE;

	/* Unblock the handling task so the task can perform any processing necessitated
	 by the interrupt.  xHandlingTask is the task's handle, which was obtained
	 when the task was created.  The handling task's notification value
	 is bitwise ORed with the interrupt status - ensuring bits that are already
	 set are not overwritten. */
	xTaskNotifyFromISR(MACTaskHandle, TIME_SLICE, eSetBits,
			&xHigherPriorityTaskWoken);

	//
	// Clear the timer interrupt flag.
	//
	TimerIntClear(TIMER4_BASE, TIMER_TIMA_TIMEOUT);

	/*
	 Force a context switch if xHigherPriorityTaskWoken is now set to pdTRUE.
	 The macro used to do this is dependent on the port and may be called
	 portEND_SWITCHING_ISR. */
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
