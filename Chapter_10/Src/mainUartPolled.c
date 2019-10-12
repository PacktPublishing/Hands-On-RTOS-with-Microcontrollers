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
#include <UartQuickDirtyInit.h>
#include "Uart4Setup.h"

/*********************************************
 * A demonstration of a polled UART driver for
 * sending and receiving
 *********************************************/


// some common variables to use for each task
// 128 * 4 = 512 bytes
//(recommended min stack size per task)
#define STACK_SIZE 128

void polledUartReceive ( void* NotUsed );
void uartPrintOutTask( void* NotUsed);

static QueueHandle_t uart2_BytesReceived = NULL;

int main(void)
{
	HWInit();
	STM_UartInit(USART2, 9600, NULL, NULL);
	SetupUart4ExternalSim();
	SEGGER_SYSVIEW_Conf();
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);	//ensure proper priority grouping for freeRTOS

	//setup tasks, making sure they have been properly created before moving on
	assert_param(xTaskCreate(polledUartReceive, "polledUartReceive", STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL) == pdPASS);
	assert_param(xTaskCreate(uartPrintOutTask, "polledUartReceive", STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL) == pdPASS);

	uart2_BytesReceived = xQueueCreate(10, sizeof(char));

//	for(int i = 0; i < 10; i++)
//	{
//		UART4->TDR = i;
//		while(!(UART4->ISR & USART_ISR_TXE));
//	}
	//start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	//if you've wound up here, there is likely an issue with overrunning the freeRTOS heap
	while(1)
	{
	}
}

void uartPrintOutTask( void* NotUsed)
{
	char nextByte;

	while(1)
	{
		xQueueReceive(uart2_BytesReceived, &nextByte, portMAX_DELAY);
		SEGGER_SYSVIEW_PrintfHost("%c", nextByte);
	}
}
/**
 * This receive task watches a queue for a new ledCmd to be added to it
 */
void polledUartReceive( void* NotUsed )
{
	uint8_t nextByte;
	//setup UART
	USART2->CR1 = USART_CR1_RE_Msk | USART_CR1_UE_Msk;
	while(1)
	{
		while(!(USART2->ISR & USART_ISR_RXNE_Msk));
		nextByte = USART2->RDR;
		xQueueSend(uart2_BytesReceived, &nextByte, 0);
	}
}

