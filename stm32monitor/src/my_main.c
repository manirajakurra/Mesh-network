/* my_main.c: main file for monitor */
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include "nrf24l01_lib.h"
#include "common.h"

/* This include will give us the CubeMX generated defines */
#include "main.h"

#define CHANNEL_ADDRESS 0xD3

uint8_t sFlag = 0;
uint8_t txActive = 0;
uint8_t txStage = 0;
uint8_t tFlag = 0;
uint8_t strLength = 0;
uint8_t intNodeID = 0;
volatile uint8_t broadcastFlag;
uint8_t messageFrom = 0;

TIM_HandleTypeDef tim17;
TIM_HandleTypeDef htim2;

//char msg[10] = "ABCDEFGHI";
uint8_t rxNodeID = 0;
uint8_t directLink = 0;

uint8_t startBeaconBroadcast = 1;

uint8_t txData1[PAYLOAD_LEN] = {99,98,97,96};

uint8_t rxData1[PAYLOAD_LEN] = {199,198,197,196};

uint8_t defaultTxAdrs[5] = {231,231,231,231,231};
uint8_t defaultRxP1Adrs[5] = {194,194,194,194,194};

uint8_t ackStage = 1;

uint8_t payloadRxdStatus = 0;
uint8_t dataAck2RceeiveStatus = 0;
uint8_t intermediateNode = 0;
uint8_t checkNeighbourNode = 0;
uint8_t destNodeID = 0;
uint8_t payloadRxd[PAYLOAD_LEN] = {0,0,0,0};
uint8_t *txDat;

uint8_t EOT = 0;//end of transmission flag


routeTable *pHead = NULL;
routeTable *newNode = NULL;

/* This function is called from the CubeMX generated main.c, after all
 * the HAL peripherals have been initialized. */
void my_init(void)
{
	/* Initialize the terminal system */
	TerminalInit();
	static uint8_t configCount = 0;

	/* Check Reset Source */
	WDTCheckReset();


	/* Initialize the task system */
	TaskingInit();
	GPIO_Init();
	spi_init();
	//configuration of nRF24L01
	printf("\n\n\n\rConfigured %d times\n\n\n\n\r",configCount);
	if(configCount == 0)
	{
		configforDypd();
	}
	configCount++;
	config_nrf24l01(Rx);
	initializeTimer17();
	initializeTimer2();
	startToBroadcastInfo();
	printf("Configuration done\n\r");
	SET_CE;
	HAL_Delay(1);
	my_Init();

}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	//extern uint8_t sFlag;

	if (GPIO_Pin == GPIO_PIN_4)
	{
		printf("Rxd Interrupt\n\r");
		sFlag = 1;
	}
}

void clearFlags()
{
	uint8_t spiCmd = 0;
	uint8_t spiData = 0;
	//STATUS Register
	// clear the RX_DR bit
	spiCmd = _NRF24L01P_SPI_CMD_WR_REG |_NRF24L01P_REG_STATUS;
	spiData = 94; //'01001110'
	send_data_to_spi(spiCmd, spiData);

	//FLUSH_TX
	spiCmd = _NRF24L01P_SPI_CMD_FLUSH_TX;
	spiData = 0; //'00000000'
	send_data_to_spi(spiCmd, spiData);

	//FLUSH_RX
	spiCmd = _NRF24L01P_SPI_CMD_FLUSH_RX;
	spiData = 0; //'00000000'
	send_data_to_spi(spiCmd, spiData);

}


/* This function is called from inside the CubeMX generated main.c,
 * inside the while(1) loop. */
void my_main(void)
{
	//extern uint8_t sFlag;
	uint8_t rxData[PAYLOAD_LEN] = {0};
	uint8_t fifoStatusRx = 0;
	static uint8_t txData[PAYLOAD_LEN] = {0,0,0,0};
	uint8_t txAdrs[5] = {194,194,194,194,211};

	static uint8_t nRetries = 0;
	uint8_t i = 0;
	uint8_t spiCmd = 0;
	uint8_t spiData = 0;
	static uint8_t masterNodeID = 0;
	uint8_t payloadLen;
	char *rxdString;

	uint8_t beaconLen = 0;

	uint8_t beaconPayload[32] = {0};

	TaskingRun();  /* Run all registered tasks */
	my_Loop();

	if(broadcastFlag && startBeaconBroadcast)
	{

		broadcastFlag = 0;
		HAL_TIM_Base_Start_IT(&htim2);
		printf("\n\n\n\r******-----------Broadcast Beacon----------********\n\n\n\n\r");

		beaconLen = packBeacon(beaconPayload, pHead);

		//deleteInActiveNode(pHead);

		txMode(beaconPayload, beaconLen);
		//deleteInActiveNode(pHead);
		changeNeighborNodeStatus(pHead);
	}



	if(tFlag)
	{
		printf("\n\n\rTimer Interrupt Occured\n\n\r");
		tFlag = 0;
		nRetries++;
		HAL_TIM_Base_Stop_IT(&tim17);
		
		if(nRetries < 2)
		{
			ackStage = 1;
			if(!txActive)
			{
				TIM17->CNT = 0;
				HAL_TIM_Base_Start_IT(&tim17);
			}
		}
		else
		{
			if(txActive)
			{
				if(txStage == 1)
				{
					ackStage = 1;
					txStage++;
				}
				else
				{
					//reset to receiver mode and abort the transmission
					//POWER DOWN MODE
					//spiCmd  = _NRF24L01P_SPI_CMD_WR_REG | _NRF24L01P_REG_CONFIG;
					//spiData = 0x38;
					//send_data_to_spi(spiCmd, spiData);
					config_nrf24l01(Rx);
					configRxAddress(defaultRxP1Adrs);				
					configTxAddress(defaultTxAdrs);
					//request other nodes to start beacon broadcast
 					txData[3] = 0;
					sendControlMsg(txData, 0x00, 0x28);
					txStage = 0;
				}
			}
			else
			{
				//reset to receiver mode and abort the transmission
				//POWER DOWN MODE
				//spiCmd  = _NRF24L01P_SPI_CMD_WR_REG | _NRF24L01P_REG_CONFIG;
				//spiData = 0x38;
				//send_data_to_spi(spiCmd, spiData);
				config_nrf24l01(Rx);
				configRxAddress(defaultRxP1Adrs);				
				configTxAddress(defaultTxAdrs);
				intermediateNode = 0;
			}
			nRetries = 0;
			checkNeighbourNode = 0;

		}

		// to resend when the message received is not correct format
		if(checkNeighbourNode)
		{

			TIM17->CNT = 0;
			HAL_TIM_Base_Start_IT(&tim17);
			sendControlMsg(txData, destNodeID, 0x01);
		}

	}

	//printf("-----sFlag -----\n\r%d\n\r", sFlag);
	if(sFlag)
	{
		HAL_TIM_Base_Stop_IT(&tim17);		
		sFlag = 0;
		//clearFlags();
		//FIFO_STATUS
		//check if there are more payload
		spiCmd = _NRF24L01P_SPI_CMD_RD_REG |_NRF24L01P_REG_STATUS;
		spiData = 0; //'01001110'
		fifoStatusRx = (((receive_data_from_spi(spiCmd, spiData) & RX_DR_MASK) == 0x40)? 0 : 1);
		if(!fifoStatusRx)
		{
			//RESET_CE;

			spiCmd = _NRF24L01P_SPI_CMD_RD_REG |_NRF24L01P_REG_FIFO_STATUS;
			spiData = 0;
			// while((receive_data_from_spi(spiCmd, spiData) & 0x0E) == 0x0E);
			spiData = receive_data_from_spi(spiCmd, spiData);

			printf("\n\n\n\r--------RX FIFO STATUS -------------\n%d\n\n\n\r", spiData);

			payloadLen = receive_payload_from_spi(rxData, PAYLOAD_LEN);

			rxdString = (char *)malloc(payloadLen+1);



			if(payloadRxdStatus) // rxd payload
			{
				// printf("-----Received Message is-----\n\r\n\r");
				for(i = 0; i < payloadLen; i++)
					*(rxdString + i) = (char) *(rxData + i);

				*(rxdString + i) = '\0';
				//printf("\n\n\n\rReceived Message is : %s\n\r\n\r", rxdString);
				// printf("\n\n\r%s\n\n\n\r", rxdString);

				for(i = 0; i < payloadLen; i++)
					payloadRxd[i] = rxData[i];

				printf("\n\n\r-------Acknowledgment for payload-------\n\n\r");
				txMode(rxData1, PAYLOAD_LEN);
				payloadRxdStatus = 0;
				printf("Received_message\n\r%s\n\r\n\r", rxdString);

				printf("\n\n\n\rReceived_message_from\n\r%d\n\r\n\r", messageFrom);

			}

			if(dataAck2RceeiveStatus)
			{
				txDat[0] = 0x05;
				txDat[1] = NODE_ID;
				txDat[2] = rxNodeID;
				strLength = 4;
				txActive = 1;
				txStage = 4;
				ackStage = 1;
				EOT = 1;
				dataAck2RceeiveStatus = 0;
			}

			if(rxData[0] == 0x06)
			{

				//update routing table
				extractNeighborNodeInfo(rxData, &pHead, payloadLen);


			}
			else if(rxData[0] == 0x08)
			{
				//stop beacon transmission
				HAL_TIM_Base_Stop_IT(&htim2);

				if(startBeaconBroadcast)
				{
					startBeaconBroadcast = 0;
					//request other nodes to stop beacon broadcast
 					txData[3] = 0;
					sendControlMsg(txData, 0x00, 0x08);
				}

			}
			else if((rxData[2] == NODE_ID) || (rxData[2] == 0))
			{
				// ackStage = 1;
				switch(rxData[0])
				{
				case 0x01: txData[3] = 0;
				sendControlMsg(txData, rxData[1], 0x21);
				messageFrom = rxData[1];
				
				//TIM17->CNT = 0;
				//HAL_TIM_Base_Start_IT(&tim17);

				break;
				case 0x02: txData[3] = 0;
				sendControlMsg(txData, rxData[1], 0x22);
				
				//TIM17->CNT = 0;
				//HAL_TIM_Base_Start_IT(&tim17);

				break;
				case 0x03: txData[3] = 0;

				masterNodeID = rxData[1];
				destNodeID = rxData[3];
				
				if(destNodeID != NODE_ID)
				{
					TIM17->CNT = 0;
					HAL_TIM_Base_Start_IT(&tim17);
					sendControlMsg(txData, destNodeID, 0x01);
					checkNeighbourNode  = 1;
				}
				else
				{
					sendControlMsg(txData, masterNodeID, 0x23);
					checkNeighbourNode  = 0;
				}
				//TIM17->CNT = 0;
				//HAL_TIM_Base_Start_IT(&tim17);

				break;
				case 0x04: 
				configRxAddress(txAdrs);
				spiCmd = _NRF24L01P_SPI_CMD_RD_REG |_NRF24L01P_REG_RX_ADDR_P1;
				readpipeAdress(spiCmd);
				txData[3] = 0;
				sendControlMsg(txData, rxData[1], 0x24);
				payloadRxdStatus = 1;
				configTxAddress(txAdrs);
				TIM17->CNT = 0;
				HAL_TIM_Base_Start_IT(&tim17);

				break;

				case 0x05: printf("\n\n\n\n\rEND OF Transmission\n\n\n\r");
				configRxAddress(defaultRxP1Adrs);
				free(rxdString);
				configTxAddress(defaultTxAdrs);
				if(intermediateNode)
				{
					txActive = 1;
					ackStage = 1;
					txStage = 0;
					rxNodeID = destNodeID;
					for(i = 0; i < PAYLOAD_LEN; i++)
						txData1[i] = payloadRxd[i];
					intermediateNode = 0;
				}
				else
				{
					//request other nodes to start beacon broadcast
 					txData[3] = 0;
					sendControlMsg(txData, 0x00, 0x28);
					//start beacon broadcast
					HAL_TIM_Base_Start_IT(&htim2);
					startBeaconBroadcast = 1;	
				}
				
				//TIM17->CNT = 0;
				//HAL_TIM_Base_Start_IT(&tim17);
				break;

				case 0x21:
					HAL_TIM_Base_Stop_IT(&tim17);

					if(!checkNeighbourNode)
					{
						txActive = 1;
						txStage = 3;
						ackStage = 1;
						if(!directLink)
						{
							txActive = 1;
							txStage = 2;
							ackStage = 1;
						}
					}
					else
					{
						intermediateNode = 1;
						checkNeighbourNode = 0;
						printf("\n\n\n\r This is an intermediate node\n\n\n\r");
						txData[3] = 0;
						sendControlMsg(txData, masterNodeID, 0x23);
					}
					break;
				case 0x22:
					HAL_TIM_Base_Stop_IT(&tim17);
					//msdelay(1000);

					rxNodeID = rxData[1];
					txActive = 1;
					txStage = 2;
					ackStage = 1;
					break;
				case 0x23:
					HAL_TIM_Base_Stop_IT(&tim17);
					txActive = 1;
					txStage = 3;
					ackStage = 1;
					break;
				case 0x24:
					HAL_TIM_Base_Stop_IT(&tim17);

					configRxAddress(txAdrs);
					configTxAddress(txAdrs);
					spiCmd = _NRF24L01P_SPI_CMD_RD_REG |_NRF24L01P_REG_TX_ADDR;

					readpipeAdress(spiCmd);
					txActive = 1;
					txStage = 4;
					ackStage = 1;
					break;
				case 0x28:
					
					//start beacon broadcast
					if(!startBeaconBroadcast)
					{
						HAL_TIM_Base_Start_IT(&htim2);
						startBeaconBroadcast = 1;
						//request other nodes to start beacon broadcast
 						txData[3] = 0;
						sendControlMsg(txData, 0x00, 0x28);
					}
				}

			}



		}
		fifoStatusRx = 1;

		clearFlags();


	}

	if(txActive && ackStage)
	{

		printf("\n\n\r----------------STAGE OF TX ---------- %d\n\n\n\r", txStage);
		switch(txStage)
		{
		case 0: txData[3] = 0;
		TIM17->CNT = 0;
		HAL_TIM_Base_Start_IT(&tim17);
		sendControlMsg(txData, rxNodeID, 0x01);
		break;
		case 1: txData[3] = 0;
		TIM17->CNT = 0;
		HAL_TIM_Base_Start_IT(&tim17);
		sendControlMsg(txData, 0x00, 0x02);
		break;
		case 2: txData[3] = destNodeID;
		//TIM17->CNT = 0;
		//HAL_TIM_Base_Start_IT(&tim17);
		sendControlMsg(txData, rxNodeID, 0x03);
		break;
		case 3: txData[3] = CHANNEL_ADDRESS;
		//TIM17->CNT = 0;
		//HAL_TIM_Base_Start_IT(&tim17);
		sendControlMsg(txData, rxNodeID, 0x04);
		break;
		case 4:
			txMode(txDat, strLength);
			TIM17->CNT = 0;
			HAL_TIM_Base_Start_IT(&tim17);
			if(EOT)
			{
				configRxAddress(defaultRxP1Adrs);

				configTxAddress(defaultTxAdrs);
				EOT = 0;
				txStage = 0;
				txActive = 0;
				free(txDat);
				printf("\n\n\n\r End Of Transmission \n\n\n\r");

			}
			else
			{
				dataAck2RceeiveStatus = 1;
			}

		}
		ackStage = 0;

	}

	WDTFeed();
}

ParserReturnVal_t CmdLed(int mode)
{
	uint32_t val,rc;

	if(mode != CMD_INTERACTIVE) return CmdReturnOk;

	rc = fetch_uint32_arg(&val);
	if(rc) {
		printf("Please supply 1 | 0 to turn on or off the LED\n");
		return CmdReturnBadParameter1;
	}

	val = val ? 1 : 0;
	HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin,val);

	return CmdReturnOk;
}

ADD_CMD("led",CmdLed,"0 | 1           Control LED")

ParserReturnVal_t CmdButton(int mode)
{
	uint32_t val;

	if(mode != CMD_INTERACTIVE) return CmdReturnOk;

	val = HAL_GPIO_ReadPin(B1_GPIO_Port,B1_Pin);

	printf("Button is %s\n",val ? "released" : "pressed");

	return CmdReturnOk;
}

ADD_CMD("button",CmdButton,"                Read Button")

