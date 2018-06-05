//#include "spi.h"
#include "nrf24l01_lib.h"
#include "stm32f3xx_hal.h"
#include "common.h"

SPI_HandleTypeDef SpiHandle;

#define MASTER_BOARD 1
#define Tx 1
#define Rx 0

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
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  //GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

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

#ifdef MASTER_BOARD
  SpiHandle.Init.Mode = SPI_MODE_MASTER;
#else
  SpiHandle.Init.Mode = SPI_MODE_SLAVE;
#endif /* MASTER_BOARD */

  if(HAL_SPI_Init(&SpiHandle) != HAL_OK)
  {
    /* Initialization Error */
    printf("Error in SPI Initialization\n\r");
  }
}

 void send_data_to_spi(uint8_t spiCmd,uint8_t spiData)
 {
	RESET_CE;
	SET_CSN;
	HAL_Delay(10);
	//SET_CE;
	RESET_CSN;

	HAL_SPI_Transmit(&SpiHandle, &spiCmd, 1, 0);
    HAL_SPI_Transmit(&SpiHandle, &spiData, 1, 0);
    printf("spiData %d\n\r", spiData);
    //HAL_SPI_Transmit(&SpiHandle, &spiData, 1, 0);
    SET_CSN;
    //RESET_CE;
 }


 uint8_t receive_data_from_spi(uint8_t spiCmd, uint8_t spiData)
 {
	RESET_CE;
	SET_CSN;
	HAL_Delay(10);
	RESET_CSN;
	//uint8_t Rxd_Data = 0;

    HAL_SPI_Transmit(&SpiHandle, &spiCmd, 1, 0);
    HAL_SPI_Transmit(&SpiHandle, &spiData, 1, 0);
    HAL_SPI_Receive(&SpiHandle, &spiCmd, 1, 0);
    printf("Status %d\n", spiData);
    HAL_SPI_Receive(&SpiHandle, &spiData, 1, 0);
    printf("Status %d\n", spiData);
    HAL_SPI_Receive(&SpiHandle, &spiData, 1, 0);
    printf("Status %d\n", spiData);
    SET_CSN;
	return spiData;
 }

 void config_nrf24l01(uint8_t Mode)
 {
	uint8_t spiCmd = 0;
	uint8_t spiData = 0;
	//configuration of nRF24L01
  //CONFIG
  //PRIM_RX = 0
  //PWR_UP = 1
  //MASK_MAX_RT = 1
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
  spiData = 0; //'00000011'
  send_data_to_spi(spiCmd, spiData);

printf("Register --- FLUSH_TX\n\r");
  //to read the content of the CONFIG register in nrf24L01 module
  spiCmd = _NRF24L01P_SPI_CMD_RD_REG | _NRF24L01P_REG_STATUS;
  spiData = 0; //'00000000'
  spiData = receive_data_from_spi(spiCmd, spiData);

  //FLUSH_RX
  spiCmd = _NRF24L01P_SPI_CMD_FLUSH_RX;
  spiData = 0; //'00000011'
  send_data_to_spi(spiCmd, spiData);

  //STATUS
  spiCmd = _NRF24L01P_SPI_CMD_WR_REG |_NRF24L01P_REG_STATUS;
  spiData = 126; //'01111110'
  send_data_to_spi(spiCmd, spiData);
  HAL_Delay(100);

printf("Register --- STATUS\n\r");
  //to read the content of the CONFIG register in nrf24L01 module
  spiCmd = _NRF24L01P_SPI_CMD_RD_REG | _NRF24L01P_REG_STATUS;
  spiData = 126; //'00000000'
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
  spiData = 0; //'00000011'
  send_data_to_spi(spiCmd, spiData);

printf("Register --- RF_CH\n\r");
  //to read the content of the CONFIG register in nrf24L01 module
  spiCmd = _NRF24L01P_SPI_CMD_RD_REG | _NRF24L01P_REG_RF_CH;
  spiData = 0; //'00000000'
  spiData = receive_data_from_spi(spiCmd, spiData);

  //RX_PW_R0
  //Set payload width as 1 Byte
  spiCmd = _NRF24L01P_SPI_CMD_WR_REG | _NRF24L01P_REG_RX_PW_P0;
  spiData = 1; //'00000001'
  send_data_to_spi(spiCmd, spiData);

printf("Register --- RX_PW_P0\n\r");
  //to read the content of the CONFIG register in nrf24L01 module
  spiCmd = _NRF24L01P_SPI_CMD_RD_REG | _NRF24L01P_REG_RX_PW_P0;
  spiData = 0; //'00000000'
  spiData = receive_data_from_spi(spiCmd, spiData);

  //RX_PW_R1
  //Set payload width as 1 Byte
  spiCmd = _NRF24L01P_SPI_CMD_WR_REG | _NRF24L01P_REG_RX_PW_P1;
  spiData = 1; //'00000001'
  send_data_to_spi(spiCmd, spiData);

printf("Register --- RX_PW_P1\n\r");
  //to read the content of the CONFIG register in nrf24L01 module
  spiCmd = _NRF24L01P_SPI_CMD_RD_REG | _NRF24L01P_REG_RX_PW_P1;
  spiData = 0; //'00000000'
  spiData = receive_data_from_spi(spiCmd, spiData);


 }

ParserReturnVal_t CmdSPIMasterTx(int action)
{
  uint8_t spiCmd = 0;
  uint8_t spiData = 0;
  uint8_t i = 0;
  

  if(action==CMD_SHORT_HELP) return CmdReturnOk;

  GPIO_Init();
  spi_init();
  //configuration of nRF24L01
  config_nrf24l01(Tx);
  printf("Configuration done\n\r");
  /* Infinite loop */
  RESET_CE;
  while (i < 5)
  {
      //W_TX_PAYLOAD
      
      spiCmd = _NRF24L01P_SPI_CMD_WR_TX_PAYLOAD;
      spiData = 12; //'00001100'
      send_data_to_spi(spiCmd, spiData);
      SET_CE;
      HAL_Delay(10);
      i++;
      RESET_CE;
  }

  return CmdReturnOk;
}

ADD_CMD("SPI_MASTER_TX",CmdSPIMasterTx,"This is used for transmitting--Master Node")


ParserReturnVal_t CmdSPIMasterRx(int action)
{
  uint8_t spiCmd = 0;
  uint8_t spiData = 0;
  //uint8_t i = 0;
  uint8_t sFlag = 1;

  if(action==CMD_SHORT_HELP) return CmdReturnOk;

  GPIO_Init();
  spi_init();
  //configuration of nRF24L01
  config_nrf24l01(Rx);
  printf("Configuration done\n\r");
  /* Infinite loop */
  SET_CE;
  HAL_Delay(1);
  while(sFlag)
  {
  //printf("Rxd Flag is%d\n\r", sFlag);
  sFlag = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4);
  }
  printf("Rxd Flag is%d\n\r", sFlag);
  spiCmd = _NRF24L01P_SPI_CMD_RD_RX_PAYLOAD;
  spiData = 0;
  spiData = receive_data_from_spi(spiCmd, spiData);
  printf("payload is %d\n\r", spiData);

  return CmdReturnOk;
}

ADD_CMD("SPI_MASTER_RX",CmdSPIMasterRx,"This is used for Receiving--Slave Node")




