#include "main.h"
#include "stm32f3xx_hal.h"
#include "common.h"



/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi2;


/* Private function prototypes -----------------------------------------------*/

static void MX_GPIO_Init(void);
static void MX_SPI2_Init(void);


uint8_t txData[2];
uint8_t rxData[2];


static void MX_SPI2_Init(void)
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

static void MX_GPIO_Init(void)
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

ParserReturnVal_t spiDataTransfer(int mode) 
{ 


  if(mode != CMD_INTERACTIVE) return CmdReturnOk;
  MX_GPIO_Init();
  MX_SPI2_Init();
  
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_SET); // Set CSN
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET); // Reset CE
  
  HAL_Delay(10);
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_RESET);// Reseting CSN 

  txData[0] = 0x05;

  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET); // Set CE

  HAL_SPI_Transmit(&hspi2,txData,1,10); // Transmit Data

  //HAL_SPI_TransmitReceive(&hspi2,txData,rxData,1,5000);

  HAL_SPI_Receive(&hspi2,&txData[1],1,10); // Receive Data

  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_SET);
 //  HAL_SPI_Receive(&hspi2,rxData,10,10);
   HAL_Delay(10);
  
  return CmdReturnOk;

} 

ADD_CMD("SPI_Transfer", spiDataTransfer, "SPI DATA Transfer")





