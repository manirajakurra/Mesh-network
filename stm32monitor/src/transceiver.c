#include "nrf24l01_lib.h"
#include "stm32f3xx_hal.h"
#include "common.h"

extern TIM_HandleTypeDef tim17;
extern TIM_HandleTypeDef htim2;
extern routeTable *pHead; 
extern uint8_t directLink;

// Function name : sendControlMsg
// Description   : Function will send control messages to the receiver.
// Parameters    : data,rxID,reqID
// Returns       : Nothing 
void sendControlMsg(uint8_t *data, uint8_t rxID, uint8_t reqID)
{
	uint8_t spiCmd = 0;
	uint8_t spiData = 0;
	uint8_t payLoadLen = 4;
	uint8_t i = 0;

	RESET_CE;
	config_nrf24l01(Tx);
	printf("Configuration done\n\r");
	/* Infinite loop */
	data[0] = reqID;
	data[1] = NODE_ID;
	data[2] = rxID;
	//data[3] = 0;

	send_payload_to_spi(data, payLoadLen);
	SET_CE;

	spiCmd = _NRF24L01P_SPI_CMD_RD_REG |_NRF24L01P_REG_STATUS;
	spiData = 0; //'01001110'
	spiData = receive_data_from_spi(spiCmd, spiData);

	printf("\n\n\n\rTX STATUS: %d\n\n\n\r", spiData);
	i++;
	RESET_CE;
	while(!sFlag);

	//STATUS Register
	// clear the TX_DS bit
	spiCmd = _NRF24L01P_SPI_CMD_WR_REG |_NRF24L01P_REG_STATUS;
	spiData = 46; //'01001110'
	send_data_to_spi(spiCmd, spiData);
	sFlag =0;
	config_nrf24l01(Rx);
	SET_CE;
}

// Function name : txMode
// Description   : function is to transmit payload to the receiver. 
// Parameters    : txData,pLength
// Returns       : Nothing 

void txMode(uint8_t *txData, uint8_t pLength)
{

	uint8_t spiCmd = 0;
	uint8_t spiData = 0;

	uint8_t i = 0;

	RESET_CE;
	config_nrf24l01(Tx);

	send_payload_to_spi(txData, pLength);
	SET_CE;
	spiCmd = _NRF24L01P_SPI_CMD_RD_REG |_NRF24L01P_REG_STATUS;
	spiData = 0; //'01001110'
	spiData = receive_data_from_spi(spiCmd, spiData);

	printf("\n\n\n\rTX STATUS: %d\n\n\n\r", spiData);
	i++;

	while(!sFlag);
	RESET_CE;
	//STATUS Register
	// clear the TX_DS bit
	spiCmd = _NRF24L01P_SPI_CMD_WR_REG |_NRF24L01P_REG_STATUS;
	spiData = 46; //'01001110'
	send_data_to_spi(spiCmd, spiData);
	sFlag =0;
	config_nrf24l01(Rx);
	SET_CE;

}


// Function name : CmdSPIMasterTx
// Description   : function will receive the message and node number from user, then it send the data to the requried node 
// Parameters    : action
// Returns       : Nothing 
ParserReturnVal_t CmdSPIMasterTx(int action)
{
	extern  uint8_t txActive;
	extern uint8_t startBeaconBroadcast;
	uint8_t i = 0;
	extern uint8_t ackStage;
	uint32_t nodeID = 0;
	extern  uint8_t rxNodeID;
	extern uint8_t txStage;
	uint8_t spiCmd =0;
	extern uint8_t EOT;
	extern uint8_t destNodeID;
	extern uint8_t *txDat;
	char *inputString;
	extern uint8_t strLength;
	uint8_t txData[PAYLOAD_LEN] = {0,0,0,0};

	HAL_StatusTypeDef rc;

	if(action==CMD_SHORT_HELP) return CmdReturnOk;

	rc = fetch_uint32_arg(&nodeID);     // fetch node ID
	fetch_string_arg(&inputString);     // fetch message from user
	strLength= strlen(inputString);
	txDat = (uint8_t *)malloc(strLength);
	for (i = 0;i < strLength; i++) 
	{
		txDat[i] = (uint8_t)inputString[i];
		//printf("the value is %d", txDat[i]);
	}

	if(rc)
	{
		printf("\n\n\rEnter Node ID of the receiver\n\r");
		return 0;
	}

	rxNodeID = (uint8_t) nodeID;

	spiCmd = _NRF24L01P_SPI_CMD_RD_REG |_NRF24L01P_REG_RX_ADDR_P1;
	printf("\n\n\n\r P1 PIPE ADress\n\n\r");
	readpipeAdress(spiCmd);


	spiCmd = _NRF24L01P_SPI_CMD_RD_REG |_NRF24L01P_REG_RX_ADDR_P0;
	printf("\n\n\n\r P0 PIPE ADress\n\n\r");
	readpipeAdress(spiCmd);



	spiCmd = _NRF24L01P_SPI_CMD_RD_REG |_NRF24L01P_REG_TX_ADDR;
	printf("\n\n\n\r P1 PIPE TX   ADress\n\n\r");
	readpipeAdress(spiCmd);

	destNodeID = rxNodeID;
	EOT = 0;
	rxNodeID = searchNodeInTable(pHead, destNodeID);
	txActive = 1;
	ackStage = 1;
	txStage = 0;
	
	//stop beacon transmission
	startBeaconBroadcast = 0;
	HAL_TIM_Base_Stop_IT(&htim2);
	//request other nodes to stop beacon broadcast
 	txData[3] = 0;
	sendControlMsg(txData, 0x00, 0x08);

	if(rxNodeID != 0)	
	{
		if(rxNodeID == destNodeID)
		{
			directLink = 1;
		}
		else
		{
			directLink = 0; 
		}
	}
	else
	{
		rxNodeID = destNodeID;
		directLink = 0;
	}
	return CmdReturnOk;
}

ADD_CMD("SPI_MASTER_TX",CmdSPIMasterTx,"This is used for transmitting--Master Node")


