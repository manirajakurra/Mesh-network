#include "main.h"
#include "stm32f3xx_hal.h"
#include "common.h"



/* Private variables ---------------------------------------------------------*/
extern SPI_HandleTypeDef hspi2;

uint8_t txData[2];
uint8_t rxData[2];
uint8_t tt[2];


void MX_SPI2_Init(void)
{

  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_4BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 LD2_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

void SPIGPIO_init()
{

  GPIO_InitTypeDef GPIO_InitStruct;

 
    /* Peripheral clock enable */
    __HAL_RCC_SPI2_CLK_ENABLE();
  
    /**SPI2 GPIO Configuration    
    PB13     ------> SPI2_SCK
    PB14     ------> SPI2_MISO
    PB15     ------> SPI2_MOSI 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN SPI2_MspInit 1 */

  /* USER CODE END SPI2_MspInit 1 */


}


ParserReturnVal_t SPI_int(int mode) 
{ 


  if(mode != CMD_INTERACTIVE) return CmdReturnOk;
  MX_GPIO_Init();
  SPIGPIO_init();
  MX_SPI2_Init();
  



  
  
  return CmdReturnOk;

} 

ADD_CMD("SPI_int", SPI_int, "SPI2 intialization")

ParserReturnVal_t SPI_DataTransfer(int mode) 
{ 


  if(mode != CMD_INTERACTIVE) return CmdReturnOk;

  int tempData = 0;
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_SET); // Set CSN
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET); // Reset CE
  
  HAL_Delay(10);
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_RESET);// Reseting CSN 

  txData[0] = 0xA0;
  //tt[0] = 12;
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET); // Set CE

  //HAL_SPI_Transmit(&hspi2,txData,1,10); // Transmit Data
  
  //HAL_SPI_Transmit(&hspi2,tt,1,10); // Transmit Data
  HAL_SPI_TransmitReceive(&hspi2,txData,rxData,1,5000);
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_SET); // Set CSN

  HAL_Delay(10);
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET); // Set CE
   HAL_Delay(10);

  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_RESET);// Reseting CSN 
  //txData[0] = 0x61;
  //HAL_SPI_Receive(&hspi2,txData,1,10); // Receive Data
  //HAL_SPI_Receive(&hspi2,rxData,1,10); // Receive Data
  tempData = (int)rxData[0];
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_SET);
 //  HAL_SPI_Receive(&hspi2,rxData,10,10);
   HAL_Delay(10);
  //printf("sent value is %d \n",txData);
  printf(" The received value is %d \n",txData[0]);
  printf("The received value is %d \n", tempData);

  
  return CmdReturnOk;

} 

ADD_CMD("SPI_Transfer", SPI_DataTransfer, "SPI DATA Transfer")



