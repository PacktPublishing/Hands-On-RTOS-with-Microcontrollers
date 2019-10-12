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

#include <UartQuickDirtyInit.h>
#include <stdint.h>
#include <string.h>

/**
 * NOTE: uint16_t is used here because of oddities with the
 * STM32f767.  There is no valid byte->byte
 */
static const uint8_t uart4Msg[] = "data from uart4";
static DMA_HandleTypeDef uart4DmaTx;

static void uart4TxDmaStartRepeat( const uint8_t* Msg, uint16_t Len );
static void uart4TxDmaSetup( void );

/**
 * Setup UART4 to repeatedly transmit a message
 * via DMA (no CPU intervention required).  This simulates
 * what would happen if there was data flowing from an
 * external off-chip source and let's us concentrate on
 * what's going on with UART2
 *
 * This is a quick and dirty setup. . .
 */
void SetupUart4ExternalSim( void )
{
	//setup DMA
	uart4TxDmaSetup();

	//GPIO pins are setup in BSP/Nucleo_F767ZI_Init
	STM_UartInit(UART4, 9600, &uart4DmaTx, NULL);

	//also enable DMA for UART4 Transmits
	UART4->CR3 |= USART_CR3_DMAT_Msk;

	uart4TxDmaStartRepeat(uart4Msg, sizeof(uart4Msg));
}

static void uart4TxDmaSetup( void )
{
	__HAL_RCC_DMA1_CLK_ENABLE();
	//no need to enable any interrupts
	HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);

	//initialize the DMA peripheral to transfer uart4Msg
	//to UART4 repeatedly
	memset(&uart4DmaTx, 0, sizeof(uart4DmaTx));
	uart4DmaTx.Instance = DMA1_Stream4;
	uart4DmaTx.Init.Channel = DMA_CHANNEL_4;				//channel 4 is for UART4 Tx
	uart4DmaTx.Init.Direction = DMA_MEMORY_TO_PERIPH;	//transfering out of memory and into the peripheral register
	uart4DmaTx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;		//no fifo
	uart4DmaTx.Init.MemBurst = DMA_MBURST_SINGLE;		//transfer 1 at a time
	uart4DmaTx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	uart4DmaTx.Init.MemInc = DMA_MINC_ENABLE;			//increment 1 byte at a time
	uart4DmaTx.Init.Mode = DMA_CIRCULAR;					//this will automatically restart the transfer at the beginning after it has finished
	uart4DmaTx.Init.PeriphBurst = DMA_PBURST_SINGLE;		//write 1 at a time to the peripheral
	uart4DmaTx.Init.PeriphInc = DMA_PINC_DISABLE;		//always keep the peripheral address the same (the Tx data register is always in the same location)
	uart4DmaTx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	uart4DmaTx.Init.Priority = DMA_PRIORITY_HIGH;
	assert_param(HAL_DMA_Init(&uart4DmaTx) == HAL_OK);

	//there's probably a "right" way of doing this through HAL. . . but HAL doesn't seem to be
	//cooperating. . .so. . .
	UART4->CR3 |= USART_CR3_DMAT_Msk;	//set the DMA transmit mode flag
}

/**
 * starts a DMA transfer to the UART4 Tx register
 * that will automatically repeat after it is finished
 * @param Msg pointer to array to transfer
 * @param Len number of elements in the array
 */
static void uart4TxDmaStartRepeat( const uint8_t* Msg, uint16_t Len )
{
	//clear the transfer complete flag to make sure our transfer starts
	UART4->ICR |= USART_ICR_TCCF;
	assert_param(HAL_DMA_Start(&uart4DmaTx, (uint32_t) Msg, (uint32_t)&(UART4->TDR), Len) == HAL_OK);
}

void DMA1_Stream4_IRQHandler(void)
{
	while(1);
//  HAL_DMA_IRQHandler(&hdma_uart4_tx);
}
