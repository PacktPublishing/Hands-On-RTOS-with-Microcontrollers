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
#include <Nucleo_F767ZI_GPIO.h>
#include <task.h>
#include <semphr.h>
#include <timers.h>
#include <SEGGER_SYSVIEW.h>
#include <Nucleo_F767ZI_Init.h>

// some common variables to use for each task
// 128 * 4 = 512 bytes
//(recommended min stack size per task)
#define STACK_SIZE 128

void oneShotCallBack( TimerHandle_t xTimer );
void repeatCallBack( TimerHandle_t xTimer );

int main(void)
{
	HWInit();
	SEGGER_SYSVIEW_Conf();
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);	//ensure proper priority grouping for freeRTOS

	TimerHandle_t repeatHandle =
		xTimerCreate(	"myRepeatTimer",			//name for timer
						500 /portTICK_PERIOD_MS,	//period of timer in ticks
						pdTRUE,						//auto-reload flag
						NULL,						//unique ID for timer
						repeatCallBack);			//callback function
	assert_param(repeatHandle != NULL);
	xTimerStart(repeatHandle, 0);

	//start with Blue LED on - it will be turned off after one-shot fires
	BlueLed.On();
	TimerHandle_t oneShotHandle =
		xTimerCreate(	"myOneShotTimer",			//name for timer
						2200 /portTICK_PERIOD_MS,	//period of timer in ticks
						pdFALSE,					//auto-reload flag
						NULL,						//unique ID for timer
						oneShotCallBack);			//callback function
	assert_param(oneShotHandle != NULL);

	xTimerStart(oneShotHandle, 0);

	//wait for the push button
	while(!ReadPushButton());

	//start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	//if you've wound up here, there is likely an issue with overrunning the freeRTOS heap
	while(1)
	{
	}
}

void oneShotCallBack( TimerHandle_t xTimer )
{
	SEGGER_SYSVIEW_PrintfHost("blue LED off");
	BlueLed.Off();
}

void repeatCallBack( TimerHandle_t xTimer )
{
	static uint32_t counter = 0;

	SEGGER_SYSVIEW_PrintfHost("toggle Green LED");
	//toggle the green LED
	if(counter++ % 2)
	{
		GreenLed.On();
	}
	else
	{
		GreenLed.Off();
	}
}
