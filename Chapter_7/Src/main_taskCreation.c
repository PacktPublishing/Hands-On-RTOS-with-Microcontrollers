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
#include <Nucleo_F767ZI_Init.h>
#include <stm32f7xx_hal.h>
#include <Nucleo_F767ZI_GPIO.h>
#include <task.h>
#include <SEGGER_SYSVIEW.h>

/**
 * 	function prototypes
 */
void GreenTask(void *argument);
void BlueTask(void *argument);
void RedTask(void *argument);
void lookBusy( void );

//the address of Task2Handle is passed to xTaskCreate.
//this global variable will be used by Task3 to delete BlueTask.
TaskHandle_t blueTaskHandle;

// some common variables to use for each task
// 128 * 4 = 512 bytes
//(recommended min stack size per task)
#define STACK_SIZE 128

//define stack and task control block (TCB) for the red task
static StackType_t RedTaskStack[STACK_SIZE];
static StaticTask_t RedTaskTCB;

int main(void)
{
	HWInit();

	//using an inlined if statement with an infinite while loop to stop in case
	//the task wasn't created successfully
	if (xTaskCreate(GreenTask, "GreenTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL) != pdPASS){ while(1); }

	//using an assert to ensure proper task creation
	assert_param(xTaskCreate(BlueTask, "BlueTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &blueTaskHandle) == pdPASS);

	//xTaskCreateStatic returns task hanlde
	//always passes since memory was statically allocated
	xTaskCreateStatic(	RedTask, "RedTask", STACK_SIZE, NULL,
						tskIDLE_PRIORITY + 1,
						RedTaskStack, &RedTaskTCB);

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

	//a task can delete itself by passing NULL to vTaskDelete
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

void RedTask( void* argument )
{
	uint8_t firstRun = 1;

	while(1)
	{
		lookBusy();

		SEGGER_SYSVIEW_PrintfHost("RedTaskRunning\n");
		RedLed.On();
		vTaskDelay(500/ portTICK_PERIOD_MS);
		RedLed.Off();
		vTaskDelay(500/ portTICK_PERIOD_MS);

		if(firstRun == 1)
		{
			//tasks can delete one-another by passing the desired
			//TaskHandle_t to vTaskDelete
			vTaskDelete(blueTaskHandle);
			firstRun = 0;
		}
	}
}

void lookBusy( void )
{
	volatile uint32_t dontCare = 0;
	for(int i = 0; i < 50E3; i++)
	{
		dontCare = i % 4;
	}
	SEGGER_SYSVIEW_PrintfHost("looking busy %d\n", dontCare);
}
