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

#include "ledTask.h"

void ledTask( void* LedPtr);

/********************************** PUBLIC *************************************/

/**
 * Initialize a task for flashing some LED's.  This task is completely started by
 * the LedTaskInit function.  It will automatically start at the desired priority
 * when the FreeRTOS scheduler is started.  It is completely independent of the
 * hardware it is controlling, so it can be resused across many different projects
 * without modification by passing in different iLed implementations.
**/
TaskHandle_t LedTaskInit(iLed* LedPtr, uint8_t Priority, uint16_t StackSize)
{
	TaskHandle_t ledTaskHandle = NULL;
	if(LedPtr == NULL){while(1);}
	if(xTaskCreate(ledTask, "ledTask", StackSize, LedPtr, Priority, &ledTaskHandle) != pdPASS){while(1);}

	return ledTaskHandle;
}

/********************************** PRIVATE *************************************/
void ledTask( void* LedPtr)
{
	iLed* led = (iLed*) LedPtr;

	while(1)
	{
		led->On();
		vTaskDelay(100);
		led->Off();
		vTaskDelay(100);
	}
}
