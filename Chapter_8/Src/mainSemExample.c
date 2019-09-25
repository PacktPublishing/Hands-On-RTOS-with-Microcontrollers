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

// some common variables to use for each task
// 128 * 4 = 512 bytes
//(recommended min stack size per task)
#define STACK_SIZE 128

void GreenTaskA( void * argument);
void BlueTaskB( void* argumet );

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

	//create TaskA as a higher priority than TaskB.  In this example, this isn't strictly necessary since the tasks
	//spend nearly all of their time blocked
	assert_param(xTaskCreate(GreenTaskA, "GreenTaskA", STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL) == pdPASS);

	//using an assert to ensure proper task creation
	assert_param(xTaskCreate(BlueTaskB, "BlueTaskB", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL) == pdPASS);

	//start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	//if you've wound up here, there is likely an issue with overrunning the freeRTOS heap
	while(1)
	{
	}
}

/**
 * Task A periodically 'gives' semaphorePtr
 * NOTES:
 * - This semaphore isn't "given" to any task specifically
 * - giving the semaphore doesn't prevent taskA from continuing to run.
 *   Notice the green LED continues to blink at all times
 */
void GreenTaskA( void* argument )
{
	uint_fast8_t count = 0;
	while(1)
	{
		//every 5 times through the loop, give the semaphore
		if(++count >= 5)
		{
			count = 0;
			SEGGER_SYSVIEW_PrintfHost("Task A (green LED) gives semPtr");
			xSemaphoreGive(semPtr);
		}
		GreenLed.On();
		vTaskDelay(100/portTICK_PERIOD_MS);
		GreenLed.Off();
		vTaskDelay(100/portTICK_PERIOD_MS);
	}
}

/**
 * wait to receive semPtr and triple blink the Blue LED
 */
void BlueTaskB( void* argument )
{
	while(1)
	{
		//'take' the semaphore with a really long timeout
		SEGGER_SYSVIEW_PrintfHost("Task B (Blue LED) attempts to take semPtr");
		if(xSemaphoreTake(semPtr, portMAX_DELAY) == pdPASS)
		{
			SEGGER_SYSVIEW_PrintfHost("Task B (Blue LED) received semPtr");
			//triple blink the Blue LED
			for(uint_fast8_t i = 0; i < 3; i++)
			{
				BlueLed.On();
				vTaskDelay(50/portTICK_PERIOD_MS);
				BlueLed.Off();
				vTaskDelay(50/portTICK_PERIOD_MS);
			}
		}
//		else
//		{
//			This is the code that will be executed if we time out waiting for
//			the semaphore to be given.  In the case of a 1 mS tick rate, a uint32_t
//			will only provide a delay of around 50 days.
//			Unless "#define INCLUDE_vTaskSuspend 1" is configured in FreeRTOSConfig.h
//		}
	}
}
