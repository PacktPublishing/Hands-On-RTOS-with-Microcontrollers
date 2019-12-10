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

#include "VirtualCommDriver.h"
#include <usb_device.h>
#include "usbd_cdc.h"
#include <FreeRTOS.h>
#include <stream_buffer.h>
#include <task.h>
#include <SEGGER_SYSVIEW.h>


/******************************** NOTE *****************************/
/* This code is NOT safe to use across multiple tasks as-is. Also, it will not
 * work properly when included by multiple source files.
 * A new copy of the static variables below will be allocated for each
 * compilation unit the file is included in
 * Some additional work is required to ensure only one copy of these variables
 * is created before this file is able to be used across more than one file.
 */
#define txBuffLen 2048
static uint8_t usbTxBuff[txBuffLen];
static StreamBufferHandle_t txStream = NULL;
static TaskHandle_t usbTaskHandle = NULL;


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
	txStream = xStreamBufferCreate( txBuffLen, 1);
	assert_param( txStream != NULL);
	assert_param(xTaskCreate(usbTask, "usbTask", 256, NULL, configMAX_PRIORITIES, &usbTaskHandle) == pdPASS);
}

/**
 * Transmit data to to USB is space is available in the stream.  If no space is available
 * then the function will return immediately.
 *
 * @param Buff	pointer to the buffer to be transmitted
 * @param Len	number of bytes to transmit
 * @returns number of bytes placed into the buffer
 */
int32_t TransmitUsbDataLossy(uint8_t const* Buff, uint16_t Len)
{
	int32_t numBytesCopied = xStreamBufferSendFromISR(	txStream, Buff, Len, NULL);

	return numBytesCopied;
}

/**
 * Similar to TransmitDataLossy, but adds a bit more convenience by copying as much data as possible
 * within 2 ticks for space
 * to become available - NOT able to be called from within an ISR
 * @param Buff	pointer to the buffer to be transmitted
 * @param Len	number of bytes to transmit
 * @returns number of bytes placed into the buffer
 */
int32_t TransmitUsbData(uint8_t const* Buff, uint16_t Len)
{
	int32_t numBytesCopied = xStreamBufferSend(	txStream, Buff, Len, 1);

	if(numBytesCopied != Len)
	{
		numBytesCopied += xStreamBufferSend(	txStream, Buff+numBytesCopied, Len-numBytesCopied, 1);
	}

	return numBytesCopied;
}

/********************************** PRIVATE *************************************/

/**
 * FreeRTOS task that takes data out of the txStream and pushes
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
		xTaskNotify( usbTaskHandle, 1, eSetValueWithOverwrite);
	}
	ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

	//setup our own callback to be called when transmission is complete
	hcdc->TxCallBack = usbTxComplete;

	//ensure the USB interrupt priority is low enough to allow for
	//FreeRTOS API calls within the ISR
	NVIC_SetPriority(OTG_FS_IRQn, 6);

	while(1)
	{
		SEGGER_SYSVIEW_PrintfHost("waiting for txStream");
		//wait forever for data to become available in the stream buffer
		//txStream.  up to txBuffLen bytes of data will be copied into
		//usbTxBuff when at least 1 byte is available
		uint8_t numBytes = xStreamBufferReceive(	txStream,
													usbTxBuff,
													txBuffLen,
													portMAX_DELAY);
		if(numBytes > 0)
		{
			SEGGER_SYSVIEW_PrintfHost("pulled %d bytes from txStream", numBytes);
			USBD_CDC_SetTxBuffer(&hUsbDeviceFS, usbTxBuff, numBytes);
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
	xTaskNotifyFromISR( usbTaskHandle, 1, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
