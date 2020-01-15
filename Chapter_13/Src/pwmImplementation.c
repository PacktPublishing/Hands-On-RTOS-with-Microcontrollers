#include <pwmImplementation.h>
#include <main.h>
#include <stm32f7xx_hal.h>

/**
 * PWM implementation supplies implementations of iPWM for the red, green, and blue
 * LED's on the Nucleo board
 */

/**
 * Initialize pins for PWM output to the 3 LED's
 * Configures each timer to provide 16 bits of PWM resolution
 * per channel : 0 - 65535 (0-100%)
 *
 * Color	Pin		Timer		Alt Func	main.h
 * Red		PB14	TIM12CH1 	AF9			LD3_Pin
 * Green	PB0		TIM3CH3		AF2			LD1_Pin
 * Blue		PB7		TIM4CH2		AF2			LD2_Pin
 */
void PWMInit( void )
{
	GPIO_InitTypeDef   GPIO_InitStruct;

	//take care of timer initialization
	TIM_HandleTypeDef    TimHandle;
	TIM_OC_InitTypeDef sConfig;

	//first, initialize all of the relevant timer clocks
	__HAL_RCC_TIM12_CLK_ENABLE();
	__HAL_RCC_TIM3_CLK_ENABLE();
	__HAL_RCC_TIM4_CLK_ENABLE();

	//init prescalar value for the counter to run at 216 Mhz
	uint32_t uhPrescalerValue = (uint32_t)((SystemCoreClock/2) / 21600000) - 1;
	TimHandle.Init.Prescaler         = uhPrescalerValue;
	TimHandle.Init.Period            = 65535;
	TimHandle.Init.ClockDivision     = 0;
	TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
	TimHandle.Init.RepetitionCounter = 0;
	TimHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	sConfig.OCMode       = TIM_OCMODE_PWM1;
	sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
	sConfig.OCFastMode   = TIM_OCFAST_DISABLE;
	sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
	sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	sConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;

	//green LED timer configuration
	TimHandle.Instance = TIM3;
	assert_param(HAL_TIM_PWM_Init(&TimHandle) == HAL_OK);
	assert_param(HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, TIM_CHANNEL_3) == HAL_OK);
	assert_param(HAL_TIM_PWM_Start(&TimHandle, TIM_CHANNEL_3) == HAL_OK);

	//blue LED timer configuration
	TimHandle.Instance = TIM4;
	assert_param(HAL_TIM_PWM_Init(&TimHandle) == HAL_OK);
	assert_param(HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, TIM_CHANNEL_2) == HAL_OK);
	assert_param(HAL_TIM_PWM_Start(&TimHandle, TIM_CHANNEL_2) == HAL_OK);

	//red LED timer configuration
	TimHandle.Instance = TIM12;
	assert_param(HAL_TIM_PWM_Init(&TimHandle) == HAL_OK);
	assert_param(HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, TIM_CHANNEL_1) == HAL_OK);
	assert_param(HAL_TIM_PWM_Start(&TimHandle, TIM_CHANNEL_1) == HAL_OK);

	// initialize all GPIO lines and map alternate functions
	// so the timer channels are output to the GPIO pins
	__HAL_RCC_GPIOB_CLK_ENABLE();

	// common GPIO settings PB14, PB0, PB7
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	//assign AF2_TIM3 LD1_Pin
	//Green LED is TIM3CH3
	GPIO_InitStruct.Pin = LD1_Pin;
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	//assign AF2_TIM4 LD2_Pin
	//Blue LED is TIM3CH3
	GPIO_InitStruct.Pin = LD2_Pin;
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	//assign AF9_TIM12 LD3_Pin
	//Red LED is TIM12CH1
	GPIO_InitStruct.Pin = LD3_Pin;
	GPIO_InitStruct.Alternate = GPIO_AF9_TIM12;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void SetBlueDuty( float DutyCycle )
{
	TIM4->CCR2 = DutyCycle/100.0 * 65535;
}
iPWM BluePWM = {SetBlueDuty};

void SetGreenDuty( float DutyCycle )
{
	TIM3->CCR3 = DutyCycle/100.0 * 65535;
}
iPWM GreenPWM = {SetGreenDuty};

void SetRedDuty( float DutyCycle )
{
	TIM12->CCR1 = DutyCycle/100.0 * 65535;
}
iPWM RedPWM = {SetRedDuty};
