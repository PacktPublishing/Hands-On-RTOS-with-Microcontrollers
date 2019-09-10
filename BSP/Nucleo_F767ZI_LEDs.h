#ifndef BSP_NUCLEO_F767ZI_LEDS_H_
#define BSP_NUCLEO_F767ZI_LEDS_H_

#include <stm32f7xx_hal.h>


//Create a typedef defining a simple function pointer
//to be used for LED's
typedef void (*GPIOFunc)(void);

//this struct holds function pointers to turn each LED
//on and off
typedef struct
{
	const GPIOFunc On;
	const GPIOFunc Off;
}LED;

extern LED BlueLed;
extern LED GreenLed;
extern LED RedLed;

#endif /* BSP_NUCLEO_F767ZI_LEDS_H_ */
