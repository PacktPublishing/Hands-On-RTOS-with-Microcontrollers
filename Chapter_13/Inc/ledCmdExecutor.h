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
#ifndef INC_LEDCMDEXECUTOR_H_
#define INC_LEDCMDEXECUTOR_H_
#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include <FreeRTOS.h>
#include <queue.h>
#include <iPWM.h>

typedef enum
{
	CMD_ALL_OFF = 0,
	CMD_ALL_ON = 1,
	CMD_SET_INTENSITY = 2,
	CMD_BLINK = 3
}LED_CMD_NUM;

/**
 * LedCmd is a single data structure that defines the intensities of
 * each of the LED's on the demo board.  Each member represents an
 * intensity of 0-100%
 */
typedef struct
{
	uint8_t cmdNum;
	float red;
	float green;
	float blue;
}LedCmd;

/**
 * define a struct that contains all of the information required for the
 * LED command executor, including the implementations of the abstract iPWM
 * interface
 */
typedef struct
{
	QueueHandle_t ledCmdQueue;	//queue containing LedCmd(s), passed by value
	iPWM * redPWM;
	iPWM * bluePWM;
	iPWM * greenPWM;
}CmdExecArgs;

void LedCmdExecution( void* Args );

#ifdef __cplusplus
 }
#endif
#endif /* INC_LEDCMDEXECUTOR_H_ */
