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
#include <UartQuickDirtyInit.h>
#include "Uart4Setup.h"
#include <stdbool.h>
#include <string.h>

/*********************************************
 * A demonstration of a simple receive-only block
 * based DMA driven UART driver
 *********************************************/


// some common variables to use for each task
// 128 * 4 = 512 bytes
//(recommended min stack size per task)
#define STACK_SIZE 128
//9600 - no errors
//115200 - no errors
//230400 - no errors
//256400 - no errors
//460800 - hit or miss, some corrupt data
//512800 - hit or miss, some corrupt data
#define BAUDRATE (256400)
void uartPrintOutTask( void* NotUsed);
void startUart4Traffic( TimerHandle_t xTimer );

static SemaphoreHandle_t rxDone = NULL;

static bool rxInProgress = false;
static uint_fast16_t rxLen = 0;

static DMA_HandleTypeDef usart2DmaRx;

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

	//start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	//if you've wound up here, there is likely an issue with overrunning the freeRTOS heap
	while(1)
	{
	}
}


/**
 * sets up DMA for USART2 reception into a single buffer.
 */
void setupUSART2DMA( void )
{
	__HAL_RCC_DMA1_CLK_ENABLE();
	
	NVIC_SetPriority(DMA1_Stream5_IRQn, 6);
	NVIC_EnableIRQ(DMA1_Stream5_IRQn);

	//initialize the DMA peripheral to transfer uart4Msg
	//to UART4 repeatedly
	memset(&usart2DmaRx, 0, sizeof(usart2DmaRx));
	usart2DmaRx.Instance = DMA1_Stream5;					//stream 5 is for USART2 Rx
	usart2DmaRx.Init.Channel = DMA_CHANNEL_4;				//channel 4 is for USART2 Rx/Tx
	usart2DmaRx.Init.Direction = DMA_PERIPH_TO_MEMORY;	//transfering out of memory and into the peripheral register
	usart2DmaRx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;		//no fifo
	usart2DmaRx.Init.MemBurst = DMA_MBURST_SINGLE;		//transfer 1 at a time
	usart2DmaRx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	usart2DmaRx.Init.MemInc = DMA_MINC_ENABLE;			//increment 1 byte at a time
	usart2DmaRx.Init.Mode = DMA_NORMAL;					//flow control mode set to normal
	usart2DmaRx.Init.PeriphBurst = DMA_PBURST_SINGLE;		//write 1 at a time to the peripheral
	usart2DmaRx.Init.PeriphInc = DMA_PINC_DISABLE;		//always keep the peripheral address the same (the RX data register is always in the same location)
	usart2DmaRx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	usart2DmaRx.Init.Priority = DMA_PRIORITY_HIGH;
	assert_param(HAL_DMA_Init(&usart2DmaRx) == HAL_OK);

	DMA1_Stream5->CR |= DMA_SxCR_TCIE;	//enable transfer complete interrupts
	USART2->CR3 |= USART_CR3_DMAR_Msk;	//set the DMA receive mode flag in the USART
}

/**
 * Start an interrupt driven receive.  This particular ISR is hard-coded
 * to give a semaphore when the reception is finished
 */
int32_t startReceiveDMA( uint8_t* Buffer, uint_fast16_t Len )
{
	if(!rxInProgress && (Buffer != NULL))
	{
		rxInProgress = true;
		rxLen = Len;

		//get the DMA peripheral ready to receive data immediately before enabling UART
		//so there is no chance of overrun
		//dma stream enable bit must be toggled before a transfer will properly restart
		__HAL_DMA_DISABLE(&usart2DmaRx);
		setupUSART2DMA();
		if(HAL_DMA_Start(&usart2DmaRx, (uint32_t)&(USART2->RDR), (uint32_t) Buffer, Len) != HAL_OK)
		{
			return -1;
		}

		//enable the UART
		//clears error flags
		USART2->ICR |= (USART_ICR_FECF | USART_ICR_PECF |
		            	USART_ICR_NCF | USART_ICR_ORECF);
		//for this specific instance, we'll avoid enabling UART interrupts for errors since
		//we'll wind up with a lot of noise on the line (the way the ISR is written will
		//cause a transfer to terminate if there are any errors are detected, rather than simply
		//continue with what data it can).  In this specific application, most of the "errors" at baudrates below
		//460800 are noise detection
//		USART2->CR3 |= (USART_CR3_EIE);	//enable error interrupts
		NVIC_SetPriority(USART2_IRQn, 6);
		NVIC_EnableIRQ(USART2_IRQn);
		USART2->CR1 |= (USART_CR1_UE);
		return 0;
	}

	return -1;
}

void startUart4Traffic( TimerHandle_t xTimer )
{
	SetupUart4ExternalSim(BAUDRATE);
}

void stopReceiveDMA( void )
{
	rxInProgress = false;
	HAL_DMA_Abort(&usart2DmaRx);
}

void uartPrintOutTask( void* NotUsed)
{
	uint8_t rxData[20];
	uint8_t expectedLen = 16;
	memset((void*)rxData, 0, 20);

	setupUSART2DMA();
	STM_UartInit(USART2, BAUDRATE, NULL, &usart2DmaRx);
	while(1)
	{
		startReceiveDMA(rxData, expectedLen);
		if(xSemaphoreTake(rxDone, 100) == pdPASS)
		{
			//NDTR is the remaining number of data items to be transferred,
			//0 signals completion
			if(DMA1_Stream5->NDTR == 0)
			{
				SEGGER_SYSVIEW_PrintfHost("received: ");
				SEGGER_SYSVIEW_Print((char*)rxData);
			}
			else
			{
				//stop the transfer on a timeout
				stopReceiveDMA();
				SEGGER_SYSVIEW_PrintfHost("Error expected %d bytes received %d", expectedLen, (expectedLen - DMA1_Stream5->NDTR));
			}
		}
		else
		{
	        stopReceiveDMA();
			SEGGER_SYSVIEW_PrintfHost("timeout");
		}
	}
}

/**
 * Given the DMA setup performed by setupUSART2DMA
 * this ISR will only execute when a DMA transfer is complete
 */
void DMA1_Stream5_IRQHandler(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	SEGGER_SYSVIEW_RecordEnterISR();

	if(rxInProgress && (DMA1->HISR & DMA_HISR_TCIF5))
	{
		rxInProgress = false;
		DMA1->HIFCR |= DMA_HIFCR_CTCIF5;
		xSemaphoreGiveFromISR(rxDone, &xHigherPriorityTaskWoken);
	}
	SEGGER_SYSVIEW_RecordExitISR();
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * This ISR should only execute when there is an error
 */
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

	//there is no need to check receive not empty, since
	//receive interrupt requests are triggering transfers
	//through DMA1 channel 4 stream 5
//	if(	USART2->ISR & USART_ISR_RXNE_Msk)

	SEGGER_SYSVIEW_RecordExitISR();
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
