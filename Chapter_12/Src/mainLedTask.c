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
#include <main.h>

/**
 * ledImplementation.c/h contains the hardware
 * implemenetations of the iLed Interface
 */
#include <ledImplementation.h>
#include <ledTask.h>

/*********************************************
 * A demo for creating a hardware agnostic task for driving LED's
 *********************************************/

int main(void)
{
	HWInit();
	SEGGER_SYSVIEW_Conf();
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);	//ensure proper priority grouping for freeRTOS
	LedTaskInit(&GreenLed, tskIDLE_PRIORITY+1, 128);
	LedTaskInit(&BlueLed, tskIDLE_PRIORITY+1, 128);
	LedTaskInit(&RedLed, tskIDLE_PRIORITY+1, 128);

	//start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	//shouldn't get here
	while(1);
}
