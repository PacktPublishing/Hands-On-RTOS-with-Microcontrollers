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

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <SEGGER_SYSVIEW.h>
#include <Nucleo_F767ZI_GPIO.h>
#include <Nucleo_F767ZI_Init.h>
#include <stm32f7xx_hal.h>

// some common variables to use for each task
// 128 * 4 = 512 bytes
//(recommended min stack size per task)
#define STACK_SIZE 128

static void blinkTwice( LED* led );
static void lookBusy( uint32_t numIterations );

void TaskA( void * argument);
void TaskB( void* argumet );
void TaskC( void* argumet );

//create storage for a pointer to a semaphore
SemaphoreHandle_t semPtr = NULL;

int main(void)
{
	HWInit();
	SEGGER_SYSVIEW_Conf();
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);	//ensure proper priority grouping for freeRTOS

	//create a semaphore using the FreeRTOS Heap
	semPtr = xSemaphoreCreateBinary();
	assert_param(semPtr != NULL);

	assert_param(xTaskCreate(TaskA, "TaskA", STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL) == pdPASS);
	assert_param(xTaskCreate(TaskB, "TaskB", STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL) == pdPASS);
	assert_param(xTaskCreate(TaskC, "TaskC", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL) == pdPASS);

	//initial "give" of the semaphore
	xSemaphoreGive(semPtr);

	//start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	//if you've wound up here, there is likely an issue with overrunning the freeRTOS heap
	while(1)
	{
	}
}

/**
 * Task A is the highest priority task in the system
 * it periodically takes the semaphore with a 200mS timeout
 * if the semaphore is taken within 200 mS, the red LED is turned off
 * and the green LED is blinked twice
 * if the semaphore is not taken within 200mS, the red LED is turned on
 * (after TaskA has been given context)
 */
void TaskA( void* argument )
{
	while(1)
	{
		//'take' the semaphore with a 200mS timeout
		SEGGER_SYSVIEW_PrintfHost("attempt to take semPtr");
		if(xSemaphoreTake(semPtr, 200/portTICK_PERIOD_MS) == pdPASS)
		{
			RedLed.Off();
			SEGGER_SYSVIEW_PrintfHost("received semPtr");
			blinkTwice(&GreenLed);
			xSemaphoreGive(semPtr);
		}
		else
		{
			//this code is called when the semaphore wasn't taken in time
			SEGGER_SYSVIEW_PrintfHost("FAILED to receive "
			                           "semphr in time");
			RedLed.On();
		}
		//sleep for a bit to let other tasks run
		vTaskDelay(StmRand(5,30));
	}
}

/**
 * this medium priority task just wakes up periodically and wastes time.
 */
void TaskB( void* argument )
{
	uint32_t counter = 0;
	while(1)
	{
		SEGGER_SYSVIEW_PrintfHost("starting iteration %u", counter++);
		vTaskDelay(StmRand(10,25));
		lookBusy(StmRand(250000, 750000));
	}
}

/**
 * the lowest priority task in the system.  This task also attempts to
 * take the semaphore every 200mS, turning off the red LED and double
 * blinking the blue LED if it was taken within 200mS
 * takes the semaphore and turning on the red LED if it hasn't
 */
void TaskC( void* argument )
{
	while(1)
	{
		//'take' the semaphore with a 200mS timeout
		SEGGER_SYSVIEW_PrintfHost("attempt to take semPtr");
		if(xSemaphoreTake(semPtr, 200/portTICK_PERIOD_MS) == pdPASS)
		{
			RedLed.Off();
			SEGGER_SYSVIEW_PrintfHost("received semPtr");
			blinkTwice(&BlueLed);
			xSemaphoreGive(semPtr);
		}
		else
		{
			//this code is called when the semaphore wasn't taken in time
			SEGGER_SYSVIEW_PrintfHost("FAILED to receive semphr in time");
			RedLed.On();
		}
	}
}

/**
 * Blink the desired LED twice
 */
static void blinkTwice( LED* led )
{
	for(uint32_t i = 0; i < 2; i++)
	{
		led->On();
		vTaskDelay(43/portTICK_PERIOD_MS);
		led->Off();
		vTaskDelay(43/portTICK_PERIOD_MS);
	}
}

/**
 * run a simple loop for numIterations
 * @param numIterations number of iterations to compute modulus
 */
static void lookBusy( uint32_t numIterations )
{
	__attribute__ ((unused)) volatile uint32_t dontCare = 0;
	for(int i = 0; i < numIterations; i++)
	{
		dontCare = i % 4;
	}
}
