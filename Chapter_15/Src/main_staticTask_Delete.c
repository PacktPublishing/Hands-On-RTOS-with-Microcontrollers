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
#include <stdlib.h>

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
 * This demo uses heap_1 for dynamic memory allocation, but also includes
 * a "statically" allocated task.  It illustrates
 * that it is possible to vTaskDelete to remove a task from the scheduler
 * that waas created using xTaskCreateStatic.
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

StackType_t GreenTaskStack[STACK_SIZE];
StaticTask_t GreenTaskTCB;

int main(void)
{
	uint32_t* mySpace = malloc(sizeof(uint32_t));
	HWInit();

	// use retVal to track the return value of xTaskCreate, issue asserts along the way
	//to stop execution if things don't go as planned
	TaskHandle_t greenHandle = NULL;

	//using an inlined if statement with an infinite while loop to stop in case
	//the task wasn't created successfully
	greenHandle = xTaskCreateStatic(	GreenTask, "GreenTask", STACK_SIZE,
										NULL, tskIDLE_PRIORITY + 2,
										GreenTaskStack, &GreenTaskTCB);
	assert_param( greenHandle != NULL);

	BaseType_t retVal = xTaskCreate(	BlueTask, "BlueTask", STACK_SIZE,
											NULL, tskIDLE_PRIORITY + 2, NULL);
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
	 * in this example, the GreenTask has been created using
	 * "static" memory.  Calling vTaskDelete will remove
	 * the task from being executed by the scheduler, but will not
	 * have any affect on the memory that was allocated for the
	 * task's stack (GreenTaskStack) or TCB (GreenTaskTCB).
	 * After a call to vTaskDelete, that memory can be safety
	 * used for other purposes, it will not longer be accessed by
	 * FreeRTOS
	 **/
	vTaskDelete(NULL);

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

