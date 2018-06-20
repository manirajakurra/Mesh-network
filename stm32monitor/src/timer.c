#include "stdio.h"
#include "common.h"
#include "stdint.h"


extern TIM_HandleTypeDef tim17;
extern uint8_t tFlag;

/*   FUNCTION      : HAL_TIM_PeriodElapsedCallback()
 *   DESCRIPTION   : Function gets executed when the timer counter gets elapsed
 *    PARAMETERS   : TIM_HandleTypeDef *htim
 *    RETURN       : none
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance==TIM17)
  {
	static uint16_t tCount = 0;
	
        tCount++;
        if(tCount > 2000)
	{
	tFlag = 1;
        tCount = 0;
       	HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);
	}
	//tCount++;
  }
 }

void initializeTimer17()
{

__HAL_RCC_TIM17_CLK_ENABLE();

  tim17.Instance = TIM17;
  tim17.Init.Prescaler = 36000 - 1;
  tim17.Init.CounterMode = TIM_COUNTERMODE_UP;
  tim17.Init.Period = 1999;
  tim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

  tim17.Init.RepetitionCounter = 0;
  HAL_NVIC_SetPriority(TIM17_IRQn, 3, 0); 
  HAL_NVIC_EnableIRQ(TIM17_IRQn);
  //HAL_TIM_Base_Start_IT(&tim17);
}
