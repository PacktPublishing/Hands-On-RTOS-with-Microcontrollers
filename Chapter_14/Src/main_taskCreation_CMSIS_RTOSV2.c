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

/**
 * This file only references RTOS-implementation independent headers.
 * Any CMSIS-RTOS v2 compliant RTOS should be able to be used
 * in the place of FreeRTOS
 *
 * Along those same lines, there is no STM32-specific code referenced
 * from this file either (although Nucleo is in the filenames, the headers
 * themselves are hardware agnostic).  This means that this code is also
 * not tied directly to the hardware.
 */
#include <Nucleo_F767ZI_Init.h>
#include <Nucleo_F767ZI_GPIO.h>
#include <cmsis_os2.h>
#include <RTOS_Dependencies.h>

/**
 * 	function prototypes
 */
void GreenTask(void *argument);
void RedTask(void *argument);
void lookBusy( void );

// some common variables to use for each task
// 512 bytes
//(recommended min stack size per task)
#define STACK_SIZE 512

//define stack the red task
static uint8_t RedTask_Stack[STACK_SIZE];

/**
 * since we don't want this code to have direct dependencies on FreeRTOS.h,
 * we can't use sizeof(StaticTask_t) directly - instead TCB_SIZE is declared in
 * Nucleo_F767ZI_Init.h and defined in Nucleo_F767ZI_Init.c
**/
uint8_t RedTask_TCB[TCB_SIZE];

int main(void)
{
	void* greenTaskThreadID = NULL;
	void* redTaskThreadID = NULL;
	osStatus_t status;
	HWInit();

	//initialize the kernel - this will setup any required
	//heap regions (if using FreeRTOS heap5)
	status = osKernelInitialize();
	while(status != osOK);

	//start the green task
	osThreadAttr_t greenThreadAtrribs = {	.name = "GreenTask",
											.attr_bits = osThreadDetached,
											.cb_mem = NULL,
											.cb_size = 0,
											.stack_mem = NULL,
											.stack_size = STACK_SIZE,
											.priority = osPriorityNormal,
											.tz_module = 0,
											.reserved = 0};
	greenTaskThreadID = osThreadNew( GreenTask, NULL, &greenThreadAtrribs);
	while(greenTaskThreadID == NULL);


	//start the red task
	//this shows how to use static allocation for the stack and task control block
	osThreadAttr_t redThreadAtrribs =  {	.name = "RedTask",
											.attr_bits = osThreadDetached,
											.cb_mem = RedTask_TCB,
											.cb_size = TCB_SIZE,
											.stack_mem = RedTask_Stack,
											.stack_size = STACK_SIZE,
											.priority = osPriorityNormal,
											.tz_module = 0,
											.reserved = 0};
	redTaskThreadID = osThreadNew( RedTask, NULL, &redThreadAtrribs);
	while(redTaskThreadID == NULL);

	status = osKernelStart();
	while(status != osOK);

	//if you've wound up here, there is likely an issue with overrunning the freeRTOS heap
	while(1)
	{
	}
}

void GreenTask(void *argument)
{
	while(1)
	{
		GreenLed.On();
		osDelay(200);
		GreenLed.Off();
		osDelay(200);
	}
}

void RedTask( void* argument )
{
	while(1)
	{
		lookBusy();
		RedLed.On();
		osDelay(500);
		RedLed.Off();
		osDelay(500);
	}
}

void lookBusy( void )
{
	volatile uint32_t dontCare;
	for(int i = 0; i < 50E3; i++)
	{
		dontCare = i % 4;
	}
}
