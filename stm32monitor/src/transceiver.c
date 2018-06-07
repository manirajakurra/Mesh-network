#include "nrf24l01_lib.h"
#include "stm32f3xx_hal.h"
#include "common.h"


uint8_t RxMode()
{
  uint8_t spiCmd = 0;
  uint8_t spiData = 0;

  spiCmd = _NRF24L01P_SPI_CMD_RD_RX_PAYLOAD;
  spiData = 0;
  spiData = receive_data_from_spi(spiCmd, spiData);

  return(spiData);
}


ParserReturnVal_t CmdSPIMasterTx(int action)
{
  uint8_t spiCmd = 0;
  uint8_t spiData = 0;
 uint8_t txData[PAYLOAD_LEN] = {11,12,13,14,15,16,17,18};
  uint8_t i = 0;
  

  if(action==CMD_SHORT_HELP) return CmdReturnOk;

  GPIO_Init();
  spi_init();
  //configuration of nRF24L01
  RESET_CE;
  config_nrf24l01(Tx);
  printf("Configuration done\n\r");
  /* Infinite loop */
  
 // while (i < 5)
  //{
      //W_TX_PAYLOAD
      
      //spiCmd = _NRF24L01P_SPI_CMD_WR_TX_PAYLOAD;
      //spiData = 12 + i; //'00001100'
      send_payload_to_spi(txData, PAYLOAD_LEN);
      SET_CE;
      //HAL_Delay(10);
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
//  }

  return CmdReturnOk;
}

ADD_CMD("SPI_MASTER_TX",CmdSPIMasterTx,"This is used for transmitting--Master Node")


ParserReturnVal_t CmdSPIMasterRx(int action)
{
  if(action==CMD_SHORT_HELP) return CmdReturnOk;

  RxMode();

  return CmdReturnOk;
}

ADD_CMD("SPI_MASTER_RX",CmdSPIMasterRx,"This is used for Receiving--Slave Node")


