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

/* This function is called from the CubeMX generated main.c, after all
 * the HAL peripherals have been initialized. */
void my_init(void)
{
  /* Initialize the terminal system */
  TerminalInit();

  /* Check Reset Source */
  WDTCheckReset();

  
  /* Initialize the task system */
  TaskingInit();
  GPIO_Init();
  spi_init();
  //configuration of nRF24L01
  config_nrf24l01(Rx);
  printf("Configuration done\n\r");
  SET_CE;
  HAL_Delay(1);
  my_Init();

}

uint8_t sFlag = 0;
uint8_t txActive = 0;

char msg[10] = "ABCDEFGHI";
uint8_t rxNodeID = 0;

uint8_t txData1[PAYLOAD_LEN] = {99,98,97,96};

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  //extern uint8_t sFlag;

  if (GPIO_Pin == GPIO_PIN_4)
  {
    /* place what you want to have happen when an interrupt has been
     * generated on pin 9 here. Just remember a best practice is to
     * keep the work you do in an isr Callback short
     */
	  //HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
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

//printf("Register --- FLUSH_TX\n\r");
  //FLUSH_TX
  spiCmd = _NRF24L01P_SPI_CMD_FLUSH_TX;
  spiData = 0; //'00000000'
  send_data_to_spi(spiCmd, spiData);

  //FLUSH_RX
  spiCmd = _NRF24L01P_SPI_CMD_FLUSH_RX;
  spiData = 0; //'00000000'
  send_data_to_spi(spiCmd, spiData);
/*
  //FIFO_STATUS
  //check if there are more payload
  spiCmd = _NRF24L01P_SPI_CMD_RD_REG |_NRF24L01P_REG_FIFO_STATUS;
  spiData = 0; //'01001110'
  fifoStatus = receive_data_from_spi(spiCmd, spiData) & RX_FIFO_EMPTY_MASK;
  //printf("FIFO STATUS %d\n\r", fifoStatus);
*/
}


/* This function is called from inside the CubeMX generated main.c,
 * inside the while(1) loop. */
void my_main(void)
{
  //extern uint8_t sFlag;
  uint8_t rxData[PAYLOAD_LEN] = {0};
  uint8_t fifoStatusRx = 0;
  uint8_t txData[PAYLOAD_LEN] = {21,22,23,24};
 uint8_t interruptinfo = 0;
  //uint8_t Rx_FIFO_EMPTY_Mask = 0x01;

  uint8_t i = 0;
  uint8_t spiCmd = 0;
  uint8_t spiData = 0;
  uint8_t txStage = 0;
  uint8_t ackStage = 1;
  uint8_t configStatus = 0;
 

  TaskingRun();  /* Run all registered tasks */
  my_Loop();
  
  //config_nrf24l01(Rx);

  //printf("-----sFlag -----\n\r%d\n\r", sFlag);
  if(sFlag)
  {
  sFlag = 0;
  //FIFO_STATUS
  //check if there are more payload
  spiCmd = _NRF24L01P_SPI_CMD_RD_REG |_NRF24L01P_REG_STATUS;
  spiData = 0; //'01001110'
  interruptinfo = receive_data_from_spi(spiCmd, spiData);

  fifoStatusRx = (((interruptinfo & RX_DR_MASK) == 0x40)? 0 : 1);
  if(!fifoStatusRx)
 {
  //RESET_CE;
  receive_payload_from_spi(rxData, PAYLOAD_LEN);
  printf("-----RxData -----\n\r\n\r");
  for(i = 0; i < PAYLOAD_LEN; i++)
    printf("%d\n\r", rxData[i]);

 if(rxData[2] == NODE_ID)
 {
  ackStage = 1;
  switch(rxData[0])
  {
	case 0x01: txData[3] = 0;
		sendControlMsg(txData, rxData[1], 0x21);
		break;
	case 0x02: txData[3] = 0;
		sendControlMsg(txData, rxData[1], 0x22);
		break;
	case 0x04: printf("\n\n\r-------Configuring data pipe2-------\n\n\r");
		configStatus = configPipe(rxData[3]);
                if(configStatus)
                 printf("\n\n\rDone\n\n\r");
		
		spiCmd = _NRF24L01P_SPI_CMD_RD_REG |_NRF24L01P_REG_RX_PW_P2;
		readpipeAdress(spiCmd);
		txData[3] = 0;
		sendControlMsg(txData, rxData[1], 0x24);
		break;
	case 0x21: 
		txActive = 1;
		txStage = 3;
		ackStage = 1;
		break;
	case 0x22: 
		txActive = 1;
		txStage++;
		ackStage = 1;
		break;
        case 0x23: 
		txActive = 1;
		txStage++;
		ackStage = 1;
		break;
   	case 0x24:
		configStatus = configPipe(CHANNEL_ADDRESS);
		configTxAddress(CHANNEL_ADDRESS);
		printf("\n\n\r-------Configuring Adress data pipe2-------\n\n\r");
		spiCmd = _NRF24L01P_SPI_CMD_RD_REG |_NRF24L01P_REG_RX_PW_P2;
		readpipeAdress(spiCmd);
		txActive = 1;
		txStage = 4;
		ackStage = 1;
		break;
		
  }
 }
 }
 fifoStatusRx = 1;

 clearFlags();
 //SET_CE;
 }

if(txActive && ackStage)
{
  switch(txStage)
  {
    case 0: txData[3] = 0;
	sendControlMsg(txData, rxNodeID, 0x01);
	break;
    case 1: txData[3] = 0;
	 sendControlMsg(txData, 0x00, 0x02);
        break;
    case 2: txData[3] = 0;
	sendControlMsg(txData, rxNodeID, 0x03);
	break;
    case 3: txData[3] = CHANNEL_ADDRESS;
        sendControlMsg(txData, 0x02, 0x04);
	break;
   case 4: 
	txMode(txData1);
        //configPipe(CHANNEL_ADDRESS);
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

