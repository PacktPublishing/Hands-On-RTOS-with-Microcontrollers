/**
 * MIT License
 *
 * Copyright (c) 2019 Brian Amos
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

/**
 * This file only references RTOS-implementation independent headers.
 * Any CMSIS-RTOS v2 compliant RTOS should be able to be used
 * in the place of FreeRTOS
 *
 * Along those same lines, there is no STM32-specific code referenced
 * from this file either (although Nucleo is in the filenames, the headers
 * themselves are hardware agnostic).  This means that this code is also
 * not tied directly to the hardware.
 */
#include <Nucleo_F767ZI_Init.h>
#include <Nucleo_F767ZI_GPIO.h>
#include <RTOS_Dependencies.h>

// FreeRTOS POSIX includes
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h>
#include <FreeRTOS_POSIX/unistd.h>

/**
 * 	function prototypes
 */
void* GreenTask(void *argument);
void* RedTask(void *argument);
void lookBusy( void );

pthread_t greenThreadId, redThreadId;

int main(void)
{
	int retVal;
	HWInit();

	//start the green task
	retVal = pthread_create( &greenThreadId, NULL, GreenTask, NULL);
	assert(retVal == 0);

	//start the red task
	retVal = pthread_create( &redThreadId, NULL, RedTask, NULL);
	assert(retVal == 0);

	vTaskStartScheduler();

	//if you've wound up here, there is likely an issue with overrunning the freeRTOS heap
	while(1)
	{
	}
}

void* GreenTask(void *argument)
{
	while(1)
	{
		GreenLed.On();
		sleep(1);
		GreenLed.Off();
		sleep(1);
	}
}

void* RedTask( void* argument )
{
	while(1)
	{
		lookBusy();
		RedLed.On();
		sleep(1);
		RedLed.Off();
		sleep(2);
	}
}

void lookBusy( void )
{
	__attribute__ ((unused)) volatile uint32_t dontCare;
	for(int i = 0; i < 50E3; i++)
	{
		dontCare = i % 4;
	}
}
