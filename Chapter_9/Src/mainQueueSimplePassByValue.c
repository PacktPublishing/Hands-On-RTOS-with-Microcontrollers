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
 *********************************************/



// some common variables to use for each task
// 128 * 4 = 512 bytes
//(recommended min stack size per task)
#define STACK_SIZE 128

/**
 * setup a simple enum to command the state of a single LED
 *
 * NOTE: only 1 LED can be commanded at a time using this method
 * THE NUMBERIC VALUES ARE IMPORTANT! These will be used by
 * sendingTask to increment the LED states
 */
typedef enum
{
	ALL_OFF	= 	0,
	RED_ON	= 	1,
	RED_OFF = 	2,
	BLUE_ON = 	3,
	BLUE_OFF= 	4,
	GREEN_ON = 	5,
	GREEN_OFF = 6,
	ALL_ON	=	7

}LED_CMDS;

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
	assert_param(xTaskCreate(recvTask, "recvTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL) == pdPASS);
	assert_param(xTaskCreate(sendingTask, "sendingTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL) == pdPASS);

	//create a queue that can store 2 uint8_t's
	//using ledCmdQueue to point to it
	ledCmdQueue = xQueueCreate(2, sizeof(uint8_t));
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
	uint8_t nextCmd = 0;

	while(1)
	{
		if(xQueueReceive(ledCmdQueue, &nextCmd, portMAX_DELAY) == pdTRUE)
		{
			switch(nextCmd)
			{
				case ALL_OFF:
					RedLed.Off();
					GreenLed.Off();
					BlueLed.Off();
				break;
				case GREEN_ON:
					GreenLed.On();
				break;
				case GREEN_OFF:
					GreenLed.Off();
				break;
				case RED_ON:
					RedLed.On();
				break;
				case RED_OFF:
					RedLed.Off();
				break;
				case BLUE_ON:
					BlueLed.On();
				break;
				case BLUE_OFF:
					BlueLed.Off();
				break;
				case ALL_ON:
					GreenLed.On();
					RedLed.On();
					BlueLed.On();
				break;
			}
		}
	}
}

/**
 * sendingTask loops through
 */
void sendingTask( void* NotUsed )
{
	while(1)
	{
		for(int i = 0; i < 8; i++)
		{
			uint8_t ledCmd = (LED_CMDS) i;
			xQueueSend(ledCmdQueue, &ledCmd, portMAX_DELAY);
			vTaskDelay(200/portTICK_PERIOD_MS);
		}
	}
}

