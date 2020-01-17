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
#include <queue.h>
#include <SEGGER_SYSVIEW.h>
#include <Nucleo_F767ZI_Init.h>

/*********************************************
 * A simple demonstration of using queues across
 * multiple tasks with pass by value.
 * This time, a large struct is copied into
 * the queue
 *********************************************/



// some common variables to use for each task
// 128 * 4 = 512 bytes
//(recommended min stack size per task)
#define STACK_SIZE 128

/**
 * define a larger structure to also specify the number of
 * milliseconds this state should last.
 * Notice the difference in ledCmdQueue->uxItemSize
 */
typedef struct
{
	uint8_t redLEDState : 1;	//specify this variable as 1 bit wide
	uint8_t blueLEDState : 1;	//specify this variable as 1 bit wide
	uint8_t greenLEDState : 1;	//specify this variable as 1 bit wide
	uint32_t msDelayTime;	//min number of mS to remain in this state
}LedStates_t;

void recvTask( void* NotUsed );
void sendingTask( void* NotUsed );

//this is a handle for the queue that will be used by
//recvTask and sendingTask
static QueueHandle_t ledCmdQueue = NULL;

int main(void)
{
	HWInit();
	SEGGER_SYSVIEW_Conf();
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);	//ensure proper priority grouping for freeRTOS

	//setup tasks, making sure they have been properly created before moving on
	assert_param(xTaskCreate(recvTask, "recvTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL) == pdPASS);
	assert_param(xTaskCreate(sendingTask, "sendingTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL) == pdPASS);

	/**
	 * create a queue that can store up to 8 copies of the struct
	 * Using sizeof allows us to modify the struct and have the queue
	 * item storage sized appropriately at compile time
	 */
	ledCmdQueue = xQueueCreate(8, sizeof(LedStates_t));
	assert_param(ledCmdQueue != NULL);

	//start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	//if you've wound up here, there is likely an issue with overrunning the freeRTOS heap
	while(1)
	{
	}
}

/**
 * This receive task watches a queue for a new ledCmd to be added to it
 */
void recvTask( void* NotUsed )
{
	LedStates_t nextCmd;

	while(1)
	{
		if(xQueueReceive(ledCmdQueue, &nextCmd, portMAX_DELAY) == pdTRUE)
		{
			if(nextCmd.redLEDState == 1)
				RedLed.On();
			else
				RedLed.Off();

			if(nextCmd.blueLEDState == 1)
				BlueLed.On();
			else
				BlueLed.Off();

			if(nextCmd.greenLEDState == 1)
				GreenLed.On();
			else
				GreenLed.Off();
		}

		vTaskDelay(nextCmd.msDelayTime/portTICK_PERIOD_MS);
	}
}

/**
 * sendingTask modifies a single nextStates variable
 * and passes it to the queue.
 * Each time the variable is passed to the queue, its
 * value is copied into the queue, which is allowed to
 * fill to capacity.
 */
void sendingTask( void* NotUsed )
{
	//a single instance of  of nextStates is defined here
	LedStates_t nextStates;

	while(1)
	{
		nextStates.redLEDState = 1;
		nextStates.greenLEDState = 1;
		nextStates.blueLEDState = 1;
		nextStates.msDelayTime = 100;

		xQueueSend(ledCmdQueue, &nextStates, portMAX_DELAY);

		nextStates.blueLEDState = 0;	//turn off just the blue LED
		nextStates.msDelayTime = 1500;
		xQueueSend(ledCmdQueue, &nextStates, portMAX_DELAY);

		nextStates.greenLEDState = 0;	//turn off just the green LED
		nextStates.msDelayTime = 200;
		xQueueSend(ledCmdQueue, &nextStates, portMAX_DELAY);

		nextStates.redLEDState = 0;
		xQueueSend(ledCmdQueue, &nextStates, portMAX_DELAY);
	}

}

