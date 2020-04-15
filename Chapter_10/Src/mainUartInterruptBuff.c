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
#include <semphr.h>
#include <timers.h>
#include <Nucleo_F767ZI_GPIO.h>
#include <SEGGER_SYSVIEW.h>
#include <Nucleo_F767ZI_Init.h>
#include <stm32f7xx_hal.h>
#include <UartQuickDirtyInit.h>
#include "Uart4Setup.h"
#include <stdbool.h>
#include <string.h>

/*********************************************
 * A demonstration of a simple receive-only block
 * based interrupt driven UART driver
 *********************************************/


// some common variables to use for each task
// 128 * 4 = 512 bytes
//(recommended min stack size per task)
#define STACK_SIZE 128

void wastefulTask( void* NotUsed);
void uartPrintOutTask( void* NotUsed);
void startUart4Traffic( TimerHandle_t xTimer );

static SemaphoreHandle_t rxDone = NULL;

static bool rxInProgress = false;
static uint_fast16_t rxLen = 0;
static uint8_t* rxBuff = NULL;
static uint_fast16_t rxItr = 0;

int main(void)
{
	HWInit();
	SEGGER_SYSVIEW_Conf();

	//ensure proper priority grouping for freeRTOS
	NVIC_SetPriorityGrouping(0);

	//setup a timer to kick off UART traffic (flowing out of UART4 TX line
	//and into USART2 RX line) 1 second after the scheduler starts
	//this delay is only present since we're using a simple
	//block-based buffer for receiving data - the transmission
	//needs to start after the receiver is ready for data for the
	//strings to start in the correct position in this simple setup
	TimerHandle_t oneShotHandle =
	xTimerCreate(	"startUart4Traffic",
					5000 /portTICK_PERIOD_MS,
					pdFALSE,
					NULL,
					startUart4Traffic);
	assert_param(oneShotHandle != NULL);
	xTimerStart(oneShotHandle, 0);

	//setup tasks, making sure they have been properly created before moving on
	rxDone = xSemaphoreCreateBinary();
	assert_param(rxDone != NULL);

	assert_param(xTaskCreate(uartPrintOutTask, "uartPrint", STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL) == pdPASS);
	assert_param(xTaskCreate(wastefulTask, "wastefulTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL) == pdPASS);

	//start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	//if you've wound up here, there is likely an issue with overrunning the freeRTOS heap
	while(1)
	{
	}
}

/**
 * Start an interrupt driven receive.  This particular ISR is hard-coded
 * to give a semaphore when the reception is finished
 */
int32_t startReceiveInt( uint8_t* Buffer, uint_fast16_t Len )
{
	if(!rxInProgress && (Buffer != NULL))
	{
		rxInProgress = true;
		rxLen = Len;
		rxBuff = Buffer;
		rxItr = 0;
		USART2->CR3 |= USART_CR3_EIE;	//enable error interrupts
		USART2->CR1 |= (USART_CR1_UE | USART_CR1_RXNEIE);
		//all 4 bits are for preemption priority -
		NVIC_SetPriority(USART2_IRQn, 6);
		NVIC_EnableIRQ(USART2_IRQn);
		return 0;
	}

	return -1;
}

void startUart4Traffic( TimerHandle_t xTimer )
{
	SetupUart4ExternalSim(9600);
}

void wastefulTask( void* NotUsed)
{
	while(1)
	{
		volatile int i, j;
		i = 10;
		j = i;
		i = j;
	}
}
void uartPrintOutTask( void* NotUsed)
{
	uint8_t rxData[20];
	uint8_t expectedLen = 16;
	memset((void*)rxData, 0, 20);

	STM_UartInit(USART2, 9600, NULL, NULL);
	while(1)
	{
		startReceiveInt(rxData, expectedLen);
		if(xSemaphoreTake(rxDone, 100) == pdPASS)
		{
			if(expectedLen == rxItr)
			{
				SEGGER_SYSVIEW_PrintfHost("received: ");
				SEGGER_SYSVIEW_Print((char*)rxData);
			}
			else
			{
				SEGGER_SYSVIEW_PrintfHost("expected %i bytes received %i", expectedLen, rxItr);
			}
		}
		else
		{
			SEGGER_SYSVIEW_PrintfHost("timeout");
		}
	}
}

void USART2_IRQHandler( void )
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	SEGGER_SYSVIEW_RecordEnterISR();

	//first check for errors
	if(	USART2->ISR & (	USART_ISR_ORE_Msk |
						USART_ISR_NE_Msk |
						USART_ISR_FE_Msk |
						USART_ISR_PE_Msk ))
	{
		//clear error flags
		USART2->ICR |= (USART_ICR_FECF |
						USART_ICR_PECF |
						USART_ICR_NCF |
						USART_ICR_ORECF);

		//if a transfer was in progress, cancel it
		//if a function pointer-based callback is used instead
		//of a semaphore, an error flag could be sent directly
		//through the function call.  With this semaphore approach,
		//the closest thing we can do is set an external error flag
		//or explicitly check the number of received bytes
		if(rxInProgress)
		{
			rxInProgress = false;
			xSemaphoreGiveFromISR(rxDone, &xHigherPriorityTaskWoken);
		}
	}

	if(	USART2->ISR & USART_ISR_RXNE_Msk)
	{
		//read the data register unconditionally to clear
		//the receive not empty interrupt if no reception is
		//in progress
		uint8_t tempVal = (uint8_t) USART2->RDR;

		if(rxInProgress)
		{
			rxBuff[rxItr++] = tempVal;
			if(rxItr >= rxLen)
			{
				rxInProgress = false;
				xSemaphoreGiveFromISR(rxDone, &xHigherPriorityTaskWoken);
			}
		}
	}
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	SEGGER_SYSVIEW_RecordExitISR();
}
