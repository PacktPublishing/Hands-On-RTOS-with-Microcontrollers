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
#include <Nucleo_F767ZI_GPIO.h>
#include <SEGGER_SYSVIEW.h>
#include <Nucleo_F767ZI_Init.h>
#include <stm32f7xx_hal.h>
#include "VirtualCommDriverMultiTask.h"
#include <string.h>
#include <stdio.h>

/*********************************************
 * A demonstration of safely sending data over USB
 * from multiple tasks
 *********************************************/


// some common variables to use for each task
// 128 * 4 = 512 bytes
//(recommended min stack size per task)
#define STACK_SIZE 128

void usbPrintOutTask( void* Number);

int main(void)
{
	HWInit();
	VirtualCommInit(256, configMAX_PRIORITIES-2);
	SEGGER_SYSVIEW_Conf();
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);	//ensure proper priority grouping for freeRTOS

	//setup tasks, making sure they have been properly created before moving on
	assert_param(xTaskCreate(usbPrintOutTask, "usbprint1", STACK_SIZE, (void*)1, tskIDLE_PRIORITY + 2, NULL) == pdPASS);
	assert_param(xTaskCreate(usbPrintOutTask, "usbprint2", STACK_SIZE, (void*)2, tskIDLE_PRIORITY + 2, NULL) == pdPASS);

	//start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	//if you've wound up here, there is likely an issue with overrunning the freeRTOS heap
	while(1)
	{
	}
}

/**
 * Define a single task which prints out a string with the argument
 * Number appended to it (for easy identification)
 */
void usbPrintOutTask( void* Number)
{
#define TESTSIZE 10
	char testString[TESTSIZE];
	memset(testString, 0, TESTSIZE);
	snprintf(testString, TESTSIZE, "task %i\n", (int) Number);
	while(1)
	{
		/**
		 * transmit the test string, waiting up to 100 mS for space to become available
		 * in the buffer
		 */
		TransmitUsbData((uint8_t*)testString, sizeof(testString), 100);
		vTaskDelay(2);
	}
}
