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
#include <queue.h>
#include <Nucleo_F767ZI_GPIO.h>
#include <SEGGER_SYSVIEW.h>
#include <Nucleo_F767ZI_Init.h>
#include "VirtualCommDriverMultiTask.h"
#include <string.h>
#include <stdio.h>
#include <pwmImplementation.h>
#include <ledCmdExecutor.h>
#include <CRC32.h>

// some common variables to use for each task
// 128 * 4 = 512 bytes
//(recommended min stack size per task)
#define STACK_SIZE 128

/**
 * the protocolTask is responsible for watching the stream buffer
 * receiving data from the PC over USB
 * It validates incoming data, populates the
 * data structure used by the LED command executor
 * and pushes commands into the command executor's queue
 */
void frameDecoder( void* NotUsed);

/**
 * the ledCmdQueue is used to pass data from the protocol decoding task to the
 * LedCmdExecutor
 */
QueueHandle_t ledCmdQueue = NULL;

int main(void)
{
	HWInit();
	PWMInit();
	VirtualCommInit(256, configMAX_PRIORITIES-1);
	SEGGER_SYSVIEW_Conf();
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);	//ensure proper priority grouping for freeRTOS

	//create a queue for LedCmd structs, capable of holding up to 4 commands
	ledCmdQueue = xQueueCreate(4, sizeof(LedCmd));
	assert_param(ledCmdQueue != NULL);

	/**
	 * create a variable that will store the arguments
	 * passed to the LED command executor
	 * this allows the logic in ledCmdExecutor to be completely
	 * independent of the underlying implementations
	 * this is a static variable so it isn't placed on the stack.  We're
	 * using aggregate initialization to guarantee that the iPWM* members won't
	 * change over time
	 */
	static CmdExecArgs ledTaskArgs;
	ledTaskArgs.ledCmdQueue = ledCmdQueue;
	ledTaskArgs.redPWM = &RedPWM;
	ledTaskArgs.greenPWM = &GreenPWM;
	ledTaskArgs.bluePWM = &BluePWM;

	//setup tasks, making sure they have been properly created before moving on
	assert_param(xTaskCreate(frameDecoder, "frameDecoder", 256, NULL, configMAX_PRIORITIES-2, NULL) == pdPASS);
	assert_param(xTaskCreate(LedCmdExecution, "cmdExec", 256, &ledTaskArgs, configMAX_PRIORITIES-2, NULL) == pdPASS);

	//start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	//if you've wound up here, there is likely an issue with overrunning the freeRTOS heap
	while(1)
	{
	}
}

/**
 * this task monitors the UBS port, decodes complete frames from the USB Rx StreamBuffer,
 * populates an LedCmd and pushes thecommand into a queue for the LedCmdExecution task to consume
 *
 * The frame consists of a delimiter byte with value 0x02 and then 4 bytes
 * representing the command number, red duty cycle , green duty cycle, and blue duty cycle,
 * To ensure the frame has been correctly detected and
 * received, a 32 bit CRC must be validated before pushing the RGB values into
 * the queue (it comes across the wire little endian)
 *
 * <STX> <Cmd> <red> <green> <blue> <CRC LSB> <CRC> <CRC> <CRC MSB>
 *
 * STX is ASCII start of text (0x02)
 */
void frameDecoder( void* NotUsed)
{
	LedCmd incomingCmd;

#define FRAME_LEN 9
	uint8_t frame[FRAME_LEN];

	while(1)
	{
		memset(frame, 0, FRAME_LEN);

		//since this is the only task receiving from the streamBuffer, we don't
		//need to acquire a mutex before accessing it
		//if more than one task was to be receiving, vcom_rxStream would require
		//protection from a mutex

		//first, ensure we received STX (0x02)
		while(frame[0] != 0x02)
		{
			xStreamBufferReceive(	*GetUsbRxStreamBuff(),
									frame,
									1,
									portMAX_DELAY);
		}

		//now receive the rest of the frame (hopefully)
		//this won't return until the remainder has bLEDeen received
		xStreamBufferReceive(	*GetUsbRxStreamBuff(),
								&frame[1],
								FRAME_LEN-1,
								portMAX_DELAY);

		//if the frame is intact, store it into the command
		//and send it to the ledCmdQueue
		if(CheckCRC(frame, FRAME_LEN))
		{
			//populate the command with current values
			incomingCmd.cmdNum = frame[1];
			incomingCmd.red = frame[2]/255.0 * 100;
			incomingCmd.green = frame[3]/255.0 * 100;
			incomingCmd.blue = frame[4]/255.0 * 100;

			//push the command to the queue
			//wait up to 100 ticks and then drop it if not added...
			xQueueSend(ledCmdQueue, &incomingCmd, 100);
		}
	}
}
