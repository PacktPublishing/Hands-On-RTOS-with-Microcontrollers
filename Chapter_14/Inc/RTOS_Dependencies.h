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

#ifndef INC_RTOS_DEPENDENCIES_H_
#define INC_RTOS_DEPENDENCIES_H_
#ifdef __cplusplus
 extern "C" {
#endif

/**
 * This file contains RTOS dependencies, so we're able to keep
 * code decoupled from the underlying RTOS implementation.
 *
 * If your code moves to a different RTOS, only another copy of
 * this file will need to be created (containing specific values for
 * the new RTOS)
 */
#include <FreeRTOS.h>
#include <assert.h>

/**
 * specify the RTOS Task Control Block size (in bytes)
 */
#define TCB_SIZE (sizeof(StaticTask_t))

#ifdef __cplusplus
 }
#endif
#endif /* INC_RTOS_DEPENDENCIES_H_ */
