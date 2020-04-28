/**
 * MIT License
 *
 * Copyright (c) 2020 Brian Amos
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
#include <Nucleo_F767ZI_Init.h>
#include <stm32f7xx_hal.h>
#include <Nucleo_F767ZI_GPIO.h>
#include <task.h>
#include <SEGGER_SYSVIEW.h>

/**
* All heap implementations for the examples in this chapter are explicitly included
* for each eclipse configuration.  individual heap implementations are located
* in MemMang\heap_x.c
* Only one heap_x.c source file can be included at a time, otherwise linker errors
* will occur due to multiple definitions of the same function.
*
* NOTE: Middleware/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c,
* 		which has been used previously for all other chapters needs to be
* 		excluded for these examples.  This has already been taken care of
* 		in the project configuration
**/

/*
 * This demo uses heap_1 for dynamic memory allocation.  With heap_1,
 * memory may only be allocated and never freed ( a hang will occur)
 *
 */

/**
 * 	function prototypes
 */
void GreenTask(void *argument);
void BlueTask(void *argument);


// some common variables to use for each task
// 128 * 4 = 512 bytes
//(recommended min stack size per task)
#define STACK_SIZE 128

//define stack and task control block (TCB) for the red task
//static StackType_t RedTaskStack[STACK_SIZE];
//static StaticTask_t RedTaskTCB;

int main(void)
{
	HWInit();

	// use retval to track the return value of xTaskCreate, issue asserts along the way
	//to stop execution if things don't go as planned
	BaseType_t retVal = pdPASS;

	//using an inlined if statement with an infinite while loop to stop in case
	//the task wasn't created successfully
	retVal = xTaskCreate(GreenTask, "GreenTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);
	assert_param( retVal == pdPASS );

	retVal = xTaskCreate(BlueTask, "BlueTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
	assert_param( retVal == pdPASS );

	//start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	//if you've wound up here, there is likely an issue with overrunning the freeRTOS heap
	while(1)
	{
	}
}

void GreenTask(void *argument)
{
	SEGGER_SYSVIEW_PrintfHost("Task1 running \
							   while Green LED is on\n");
	GreenLed.On();
	vTaskDelay(1500/ portTICK_PERIOD_MS);
	GreenLed.Off();

	/**
	 * attempting to call vTaskDelete (or any other *Delete function
	 * i.e. vQueueDelete, vEventGroupDelete, vSemaphoreDelete, vMutexDelete, etc)
	 * will result in a hang.
	 * To observe this, uncomment the following line - you'll notice that the
	 * blue LED stops blinking.  Using a debugger, you'll also notice that
	 * the BlueTask and Software Timer task are both stopped in portYIELD_WITHIN_API
	 *
	 * If using heap_1.c  - it is best to adjust the FreeRTOSConfig.h to disable code
	 * for vTaskDelete as follows:
	 * #define INCLUDE_vTaskDelete 0
	 *
	 * This allows the linker to issue an undefined reference error when
	 * vTaskDelete used.  This allows the issue to be found at link time, rather
	 * than run time.
	 **/
//	vTaskDelete(NULL);

	// if a task should not longer run when using heap_1 an infinite
	// delay can be used instead of vTaskDelete
	while(1)
	{
		vTaskDelay(portMAX_DELAY);
	}

	//task never get's here
	GreenLed.On();
}

void BlueTask( void* argument )
{
	while(1)
	{
		SEGGER_SYSVIEW_PrintfHost("BlueTaskRunning\n");
		BlueLed.On();
		vTaskDelay(200 / portTICK_PERIOD_MS);
		BlueLed.Off();
		vTaskDelay(200 / portTICK_PERIOD_MS);
	}
}

