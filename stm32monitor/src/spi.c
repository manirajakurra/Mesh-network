//#include "spi.h"
#include "nrf24l01_lib.h"
#include "stm32f3xx_hal.h"
#include "common.h"

SPI_HandleTypeDef SpiHandle;

// Function name : configforDypd
// Description   : This fucntion is to configure NRF24L01 for dynamic payload
// Parameters    : Nothing
// Returns       : Nothing

void configforDypd()

{
	uint8_t spiCmd = 0;
	uint8_t spiData = 0;
	//POWER DOWN MODE
	spiCmd  = _NRF24L01P_SPI_CMD_WR_REG | _NRF24L01P_REG_CONFIG;
	spiData = 0x38;
	send_data_to_spi(spiCmd, spiData);

	// Activate
	spiCmd  = _NRF24L01P_SPI_CMD_ACTIVATE;
	spiData = 0x73;
	send_data_to_spi(spiCmd, spiData);

	// feature
	spiCmd  =  _NRF24L01P_SPI_CMD_WR_REG | _NRF24L01P_REG_FEATURE;
	spiData = 0x04; // 00000100
	send_data_to_spi(spiCmd, spiData);

	// DYPD
	spiCmd = _NRF24L01P_SPI_CMD_WR_REG | _NRF24L01P_REG_DYNPD;
	spiData = 0x07; //00000111;
	send_data_to_spi(spiCmd, spiData);
}

// Function name : GPIO_Init
// Description   : This function configure all the gpio pins
// Parameters    : Nothing
// Returns       : Nothing

void GPIO_Init(void)
{

	GPIO_InitTypeDef GPIO_InitStruct;

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);

	/*Configure GPIO pin : PB3 */
	GPIO_InitStruct.Pin = GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : PA0 */
	//CE
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : PA1 */
	//CSN
	GPIO_InitStruct.Pin = GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : PA4 */
	//CSN
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	//GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : PA0 */
	//CE
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	HAL_NVIC_SetPriority(EXTI4_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);

}

// Function name : spi_init
// Description   : This fucntion is to intialize SPI 3
// Parameters    : Nothing
// Returns       : Nothing

void spi_init(void)
{
	GPIO_Init();
	/*##-1- Configure the SPI peripheral #######################################*/
	/* Set the SPI parameters */
	SpiHandle.Instance               = SPIx;
	SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
	SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
	SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
	SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
	SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
	SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
	SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;
	SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
	SpiHandle.Init.CRCPolynomial     = 7;
	SpiHandle.Init.CRCLength         = SPI_CRC_LENGTH_8BIT;
	SpiHandle.Init.NSS               = SPI_NSS_SOFT;
	SpiHandle.Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;

	SpiHandle.Init.Mode = SPI_MODE_MASTER;


	if(HAL_SPI_Init(&SpiHandle) != HAL_OK)
	{
		/* Initialization Error */
		printf("Error in SPI Initialization\n\r");
	}
}

// Function name : send_payload_to_spi
// Description   : This fucntion is to send payload to nrf module through spi
// Parameters    : payload, payloadlen
// Returns       : Nothing

void send_payload_to_spi(uint8_t * payload, uint8_t payloadLen)
{
	uint8_t i = 0;
	uint8_t spiCmd = _NRF24L01P_SPI_CMD_WR_TX_PAYLOAD;
	//RESET_CE;
	SET_CSN;
	HAL_Delay(10);
	//SET_CE;
	RESET_CSN;

	HAL_SPI_Transmit(&SpiHandle, &spiCmd, 1, 0);

	for(i = 0; i < payloadLen; i++)
	{
		HAL_SPI_Transmit(&SpiHandle, (payload+i), 1, 0);
		printf("payload %d\n\r", payload[i]);
	}
	//HAL_SPI_Transmit(&SpiHandle, &spiData, 1, 0);
	SET_CSN;
}

// Function name : send_data_to_spi
// Description   : This fucntion is to send data to configure nrf module.
// Parameters    : spiCMD, spidata
// Returns       : Nothing


void send_data_to_spi(uint8_t spiCmd,uint8_t spiData)
{
	//RESET_CE;
	SET_CSN;
	HAL_Delay(10);
	//SET_CE;
	RESET_CSN;

	HAL_SPI_Transmit(&SpiHandle, &spiCmd, 1, 0);
	HAL_SPI_Transmit(&SpiHandle, &spiData, 1, 0);
	printf("spiData %d\n\r", spiData);

	SET_CSN;
	//RESET_CE;
}
// Function name : receive_data_from_spi
// Description   : This fucntion is to receive data from nrf Module 
// Parameters    : spiCMD, spidata
// Returns       : spidata, returns data from spi

uint8_t receive_data_from_spi(uint8_t spiCmd, uint8_t spiData)
{
	//RESET_CE;
	SET_CSN;
	HAL_Delay(10);
	RESET_CSN;
	//uint8_t Rxd_Data = 0;

	HAL_SPI_Transmit(&SpiHandle, &spiCmd, 1, 0);
	HAL_SPI_Transmit(&SpiHandle, &spiData, 1, 0);
	HAL_SPI_Receive(&SpiHandle, &spiCmd, 1, 0);
	// HAL_Delay(1);
	printf("Status %d\n\r", spiData);

	HAL_SPI_Receive(&SpiHandle, &spiData, 1, 0);
	//HAL_Delay(1);
	printf("Status %d\n\r", spiData);

	HAL_SPI_Receive(&SpiHandle, &spiData, 1, 0);
	//HAL_Delay(1);
	printf("Status %d\n\r", spiData);

	SET_CSN;
	return spiData;
}
// Function name : receive_payload_from_spi
// Description   : This fucntion is to receive payload from nrf Module.
// Parameters    : payload, payloadLen
// Returns       : payloadLen, returns payloadlength 

uint8_t receive_payload_from_spi(uint8_t * payload, uint8_t payloadLen)
{
	uint8_t spiCmd = 0;

	uint8_t spiData[payloadLen];
	uint8_t i= 0;

	spiCmd = _NRF24L01P_SPI_CMD_R_RX_PL_WID;
	payloadLen = receive_data_from_spi(spiCmd, spiData[0]);

	printf("\n\n\n\rPayload Length %d\n\n\n\n\r", payloadLen);

	spiCmd = _NRF24L01P_SPI_CMD_RD_RX_PAYLOAD;

	//RESET_CE;
	SET_CSN;
	HAL_Delay(10);
	RESET_CSN;
	//uint8_t Rxd_Data = 0;

	HAL_SPI_Transmit(&SpiHandle, &spiCmd, 1, 0);
	HAL_SPI_Transmit(&SpiHandle, spiData, 1, 0);
	HAL_SPI_Receive(&SpiHandle, spiData, 1, 0);
	printf("SL %d %d\n\r", *(spiData), i);
	HAL_SPI_Receive(&SpiHandle, spiData, 1, 0);
	printf("SL %d %d\n\r", *(spiData), i);

	for(i = 0; i<payloadLen; i++)
	{
		HAL_SPI_Receive(&SpiHandle, (payload+i), 1, 0);
		// HAL_Delay(1);
		printf("PL %d %d\n\r", *(payload+i), i);
	}
	SET_CSN;

	return(payloadLen);
}

// Function name : config_nrf24l01
// Description   : This fucntion is to configure nrf Module 
// Parameters    : mode (Transmiter or Receiver) 
// Returns       : nothing 

void config_nrf24l01(uint8_t Mode)
{
	uint8_t spiCmd = 0;
	uint8_t spiData = 0;
	//configuration of nRF24L01

	if(Mode == Tx)
	{
		//SET_CE;
		printf("Configuration for TX\n\r");
		spiCmd = _NRF24L01P_SPI_CMD_WR_REG | _NRF24L01P_REG_CONFIG;
		spiData = 26; //'00011010'
		send_data_to_spi(spiCmd, spiData);
	}
	if(Mode == Rx)
	{
		//SET_CE;
		printf("Configuration for RX\n\r");
		spiCmd = _NRF24L01P_SPI_CMD_WR_REG | _NRF24L01P_REG_CONFIG;
		spiData = 59; //'00111011'
		send_data_to_spi(spiCmd, spiData);
	}

	printf("Register --- CONFIG\n\r");
	//to read the content of the CONFIG register in nrf24L01 module
	spiCmd = _NRF24L01P_SPI_CMD_RD_REG | _NRF24L01P_REG_CONFIG;
	spiData = 0; //'00000000'
	spiData = receive_data_from_spi(spiCmd, spiData);

	//EN_AA
	// enable auto ACK
	spiCmd = _NRF24L01P_SPI_CMD_WR_REG | _NRF24L01P_REG_EN_AA;
	spiData = 0; //'00111111'
	send_data_to_spi(spiCmd, spiData);

	printf("Register --- EN_AA\n\r");
	//to read the content of the CONFIG register in nrf24L01 module
	spiCmd = _NRF24L01P_SPI_CMD_RD_REG | _NRF24L01P_REG_EN_AA;
	spiData = 0; //'00000000'
	spiData = receive_data_from_spi(spiCmd, spiData);

	//EN_RXADDR
	//Enable data pipe 0 and 1
	spiCmd = _NRF24L01P_SPI_CMD_WR_REG | _NRF24L01P_REG_EN_RXADDR;
	spiData = 3; //'00000011'
	send_data_to_spi(spiCmd, spiData);

	//FLUSH_TX
	spiCmd = _NRF24L01P_SPI_CMD_FLUSH_TX;
	spiData = 0; //'00000000'
	send_data_to_spi(spiCmd, spiData);

	printf("Register --- FLUSH_TX\n\r");
	//to read the content of the CONFIG register in nrf24L01 module
	spiCmd = _NRF24L01P_SPI_CMD_RD_REG | _NRF24L01P_REG_STATUS;
	spiData = 0; //'00000000'
	spiData = receive_data_from_spi(spiCmd, spiData);

	//FLUSH_RX
	spiCmd = _NRF24L01P_SPI_CMD_FLUSH_RX;
	spiData = 0; //'00000000'
	send_data_to_spi(spiCmd, spiData);

	//STATUS
	spiCmd = _NRF24L01P_SPI_CMD_WR_REG |_NRF24L01P_REG_STATUS;
	spiData = 126; //'01111110'
	send_data_to_spi(spiCmd, spiData);
	HAL_Delay(100);

	printf("Register --- STATUS\n\r");
	//to read the content of the CONFIG register in nrf24L01 module
	spiCmd = _NRF24L01P_SPI_CMD_RD_REG | _NRF24L01P_REG_STATUS;
	spiData = 0; //'00000000'
	spiData = receive_data_from_spi(spiCmd, spiData);

	printf("Register --- RX_ADDR_P0\n\r");
	//to read the content of the CONFIG register in nrf24L01 module
	spiCmd = _NRF24L01P_SPI_CMD_RD_REG | _NRF24L01P_REG_RX_ADDR_P0;
	spiData = 0; //'00000000'
	spiData = receive_data_from_spi(spiCmd, spiData);

	printf("Register --- TX_ADDR\n\r");
	//to read the content of the CONFIG register in nrf24L01 module
	spiCmd = _NRF24L01P_SPI_CMD_RD_REG | _NRF24L01P_REG_TX_ADDR;
	spiData = 0; //'00000000'
	spiData = receive_data_from_spi(spiCmd, spiData);

	//RF_CH
	//Use channel 0
	spiCmd = _NRF24L01P_SPI_CMD_WR_REG | _NRF24L01P_REG_RF_CH;
	spiData = 0; //'00000000'
	send_data_to_spi(spiCmd, spiData);

	printf("Register --- RF_CH\n\r");
	//to read the content of the CONFIG register in nrf24L01 module
	spiCmd = _NRF24L01P_SPI_CMD_RD_REG | _NRF24L01P_REG_RF_CH;
	spiData = 0; //'00000000'
	spiData = receive_data_from_spi(spiCmd, spiData);

	//RX_PW_R0
	//Set payload width as 1 Byte
	spiCmd = _NRF24L01P_SPI_CMD_WR_REG | _NRF24L01P_REG_RX_PW_P0;
	spiData = PAYLOAD_LEN; //'00000001'
	send_data_to_spi(spiCmd, spiData);

	printf("Register --- RX_PW_P0\n\r");
	//to read the content of the CONFIG register in nrf24L01 module
	spiCmd = _NRF24L01P_SPI_CMD_RD_REG | _NRF24L01P_REG_RX_PW_P0;
	spiData = 0; //'00000000'
	spiData = receive_data_from_spi(spiCmd, spiData);

	//RX_PW_R1
	//Set payload width as 1 Byte
	spiCmd = _NRF24L01P_SPI_CMD_WR_REG | _NRF24L01P_REG_RX_PW_P1;
	spiData = PAYLOAD_LEN; //'00000001'
	send_data_to_spi(spiCmd, spiData);

	printf("Register --- RX_PW_P1\n\r");
	//to read the content of the CONFIG register in nrf24L01 module
	spiCmd = _NRF24L01P_SPI_CMD_RD_REG | _NRF24L01P_REG_RX_PW_P1;
	spiData = 0; //'00000000'
	spiData = receive_data_from_spi(spiCmd, spiData);


}

// Function name : configRxAddress
// Description   : This fucntion is to configure receiver pipe address.
// Parameters    : Adrs
// Returns       : nothing 
void configRxAddress(uint8_t * Adrs)
{
	uint8_t i = 0;
	uint8_t spiCmd = _NRF24L01P_SPI_CMD_WR_REG | _NRF24L01P_REG_RX_ADDR_P1;
	//RESET_CE;
	SET_CSN;
	HAL_Delay(10);
	//SET_CE;
	RESET_CSN;

	HAL_SPI_Transmit(&SpiHandle, &spiCmd, 1, 0);

	for(i = 0; i < 5; i++)
	{
		HAL_SPI_Transmit(&SpiHandle, (Adrs+i), 1, 0);
		printf("payload %d\n\r", Adrs[i]);
	}
	//HAL_SPI_Transmit(&SpiHandle, &spiData, 1, 0);
	SET_CSN;
}

// Function name : configTxAddress
// Description   : This fucntion is to configure transmiter pipe address. 
// Parameters    : Adrs
// Returns       : nothing 


void configTxAddress(uint8_t * Adrs)
{
	uint8_t i = 0;
	uint8_t spiCmd = _NRF24L01P_SPI_CMD_WR_REG | _NRF24L01P_REG_TX_ADDR;
	//RESET_CE;
	SET_CSN;
	HAL_Delay(10);
	//SET_CE;
	RESET_CSN;

	HAL_SPI_Transmit(&SpiHandle, &spiCmd, 1, 0);

	for(i = 0; i < 5; i++)
	{
		HAL_SPI_Transmit(&SpiHandle, (Adrs+i), 1, 0);
		printf("payload %d\n\r", Adrs[i]);
	}
	//HAL_SPI_Transmit(&SpiHandle, &spiData, 1, 0);
	SET_CSN;
}

// Function name : readpipeAdress
// Description   : This fucntion is to read the pipe address
// Parameters    : spiCmd
// Returns       : nothing 
void readpipeAdress(uint8_t spiCmd)
{
	// uint8_t spiCmd = 0;
	uint8_t spiData = 0;
	uint8_t i= 0;
	uint8_t pipeAdr[5] = {0,0,0,0,0};

	SET_CSN;
	HAL_Delay(10);
	RESET_CSN;
	//uint8_t Rxd_Data = 0;

	HAL_SPI_Transmit(&SpiHandle, &spiCmd, 1, 0);
	HAL_SPI_Transmit(&SpiHandle, &spiData, 1, 0);
	HAL_SPI_Receive(&SpiHandle, &spiData, 1, 0);
	printf("SL %d %d\n\r", spiData, i);
	HAL_SPI_Receive(&SpiHandle, &spiData, 1, 0);
	printf("SL %d %d\n\r", spiData, i);

	for(i = 0; i<5; i++)
	{
		HAL_SPI_Receive(&SpiHandle, (pipeAdr+i), 1, 0);
		// HAL_Delay(1);
		printf("P2 Adrs %d %d\n\r", *(pipeAdr+i), i);
	}
	SET_CSN;
}



