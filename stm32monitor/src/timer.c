#include "stdio.h"
#include "common.h"
#include "stdint.h"


extern TIM_HandleTypeDef tim17;
extern TIM_HandleTypeDef htim2;
extern uint8_t tFlag;
extern volatile uint8_t broadcastFlag;

static int idelay = 0;

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
		}

	}
	if (htim->Instance==TIM2)
	{
		idelay++;

		if(idelay == timerDelay)
		{
			HAL_TIM_Base_Stop_IT(&htim2);
			HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);
			printf("\n\n\r Timer2\n\n\r");
			idelay = 0;
			broadcastFlag = 1;
		}
	}

}
/*   FUNCTION      : initializeTimer17
 *   DESCRIPTION   : Function is to intialize timer 17
 *    PARAMETERS   : Nothing
 *    RETURN       : none
 */


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
}

/*   FUNCTION      : initializeTimer2
 *   DESCRIPTION   : Function is to intializeTimer timer 2
 *    PARAMETERS   : Nothing
 *    RETURN       : none
 */

void initializeTimer2()
{
	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_MasterConfigTypeDef sMasterConfig;
	__HAL_RCC_TIM2_CLK_ENABLE();
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 71;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 999;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
	{
		printf("Timer configuration failed\n\r");
	}

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
	{
		printf("Timer configuration failed\n\r");
	}

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
	{
		printf("Timer configuration failed\n\r");
	}
	HAL_NVIC_SetPriority(TIM2_IRQn, 4, 0);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

/*   FUNCTION      : startToBroadcastInfo
 *   DESCRIPTION   : function trigger to boardcast for every 45 seconds
 *    PARAMETERS   : Nothing
 *    RETURN       : none
 */

void startToBroadcastInfo()
{
	timerDelay = 45000;
	HAL_TIM_Base_Start_IT(&htim2);
	broadcastFlag = 0;
}


