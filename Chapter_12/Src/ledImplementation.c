#include <ledImplementation.h>

#include <stm32f7xx_hal.h>

/**
 * LED implementation for Red, Green, and Blue discrete LED's on
 * Nucleo-F767ZI
 */

void GreenOn ( void ) {HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);}
void GreenOff ( void ) {HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);}
iLed GreenLed = { GreenOn, GreenOff };

void BlueOn ( void ) {HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);}
void BlueOff ( void ) {HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);}
iLed BlueLed = { BlueOn, BlueOff };

void RedOn ( void ) {HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);}
void RedOff ( void ) {HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);}
iLed RedLed = { RedOn, RedOff };
