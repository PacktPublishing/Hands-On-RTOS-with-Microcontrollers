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

#include "VirtualCommDriverMultiTask.h"
#include <usb_device.h>
#include "usbd_cdc.h"
#include <FreeRTOS.h>
#include <stream_buffer.h>
#include <task.h>
#include <semphr.h>
#include <SEGGER_SYSVIEW.h>

#define txBuffLen 2048
uint8_t vcom_usbTxBuff[txBuffLen];
StreamBufferHandle_t vcom_txStream = NULL;
TaskHandle_t vcom_usbTaskHandle = NULL;
SemaphoreHandle_t vcom_mutexPtr = NULL;


//hUsbDeviceFS defined in usb_device.c
extern USBD_HandleTypeDef hUsbDeviceFS;

void usbTask( void* NotUsed);
void usbTxComplete( void );

/********************************** PUBLIC *************************************/

/**
 * Initialize the USB peripheral and HAL-based USB stack.
 * A transmit task, responsible for pulling data out of the stream buffer and
 * pushing it into the USB peripheral is also created.
**/
void VirtualCommInit( void )
{
	MX_USB_DEVICE_Init();
	vcom_txStream = xStreamBufferCreate( txBuffLen, 1);
	assert_param( vcom_txStream != NULL);

	vcom_mutexPtr = xSemaphoreCreateMutex();
	assert_param(vcom_mutexPtr != NULL);
	assert_param(xTaskCreate(usbTask, "usbTask", 256, NULL, configMAX_PRIORITIES, &vcom_usbTaskHandle) == pdPASS);
}

/**
 * Attempt to transmit Len bytes of data pointed to by Buff.  Waiting no longer
 * than DelayMs returning the number of bytes queued for transmission
 * @param Buff pointer to the buffer containing bytes to transmit
 * @param Len number bytes to transmit
 * @param DelayMs number of milliseconds to wait for space in the stream buffer
 * 		  to become available
 * @returns number of bytes added to the stream buffer
 */
int32_t TransmitUsbData(uint8_t const*  Buff, uint16_t Len, int32_t DelayMs)
{
	int32_t numBytesCopied = 0;

	//convert mS into ticks to work in native units
	const uint32_t delayTicks = DelayMs / portTICK_PERIOD_MS;
	const uint32_t startingTime = xTaskGetTickCount();
	uint32_t endingTime = startingTime + delayTicks;

	if(xSemaphoreTake(vcom_mutexPtr, delayTicks ) == pdPASS)
	{
		uint32_t remainingTime = endingTime - xTaskGetTickCount();
		numBytesCopied = xStreamBufferSend(	vcom_txStream, Buff, Len, remainingTime);

		if(numBytesCopied != Len)
		{
			remainingTime = endingTime - xTaskGetTickCount();
			numBytesCopied += xStreamBufferSend(	vcom_txStream, Buff+numBytesCopied, Len-numBytesCopied, remainingTime);
		}

		xSemaphoreGive(vcom_mutexPtr);
	}

	return numBytesCopied;
}

/********************************** PRIVATE *************************************/

/**
 * FreeRTOS task that takes data out of the vcom_txStream and pushes
 * data to be transmitted into the USB HAL buffer.
 *
 * This function waits for a task notification, which is sent by a callback generated
 * from the USB stack upon completion of a transmission
 *
 * It then then copies up to 2KB of data from the stream buffer to a
 * local buffer, which is passed to the HAL USB stack.
 *
 */
void usbTask( void* NotUsed)
{
	USBD_CDC_HandleTypeDef *hcdc = NULL;

	while(hcdc == NULL)
	{
		hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
		vTaskDelay(10);
	}
	if (hcdc->TxState == 0)
	{
		//if there is no TX in progress, immediately send a task notification
		//to kick things off
		xTaskNotify( vcom_usbTaskHandle, 1, eSetValueWithOverwrite);
	}
	ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

	//setup our own callback to be called when transmission is complete
	hcdc->TxCallBack = usbTxComplete;

	//ensure the USB interrupt priority is low enough to allow for
	//FreeRTOS API calls within the ISR
	NVIC_SetPriority(OTG_FS_IRQn, 6);

	while(1)
	{
		SEGGER_SYSVIEW_PrintfHost("waiting for vcom_txStream");
		//wait forever for data to become available in the stream buffer
		//vcom_txStream.  up to txBuffLen bytes of data will be copied into
		//vcom_usbTxBuff when at least 1 byte is available
		uint8_t numBytes = xStreamBufferReceive(	vcom_txStream,
													vcom_usbTxBuff,
													txBuffLen,
													portMAX_DELAY);
		if(numBytes > 0)
		{
			SEGGER_SYSVIEW_PrintfHost("pulled %d bytes from vcom_txStream", numBytes);
			USBD_CDC_SetTxBuffer(&hUsbDeviceFS, vcom_usbTxBuff, numBytes);
			USBD_CDC_TransmitPacket(&hUsbDeviceFS);
			//wait forever for a notification, clearing it to 0 when received
			ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
			SEGGER_SYSVIEW_PrintfHost("tx complete");
		}
	}
}

void usbTxComplete( void )
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	xTaskNotifyFromISR( vcom_usbTaskHandle, 1, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
