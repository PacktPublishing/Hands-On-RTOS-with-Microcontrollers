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
#ifndef BSP_IPWM_H_
#define BSP_IPWM_H_
#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

 /**
  * Create a typedef defining a simple function pointer
  * to be used for setting PWM via duty cycle
  *
  * This function is meant to provide a consistent and easy-to-use
  * interface to the underlying PWM implementation
  *
  * The exact details of PWM (alignment, complimentary output, etc)
  * is determined by the code providing an implementation of this function

  * @param DutyCycle 0-100 to set the PWM channel from 0% on-time to 100% on-time
  **/
 typedef void (*iPwmDutyCycleFunc)( float DutyCycle );

 /**
  * This struct definition holds function pointers to interact with an
  * LED via pulse width modulation
  */
 typedef struct
 {
 	const iPwmDutyCycleFunc SetDutyCycle;

 }iPWM;

#ifdef __cplusplus
 }
#endif
#endif /* BSP_IPWM_H_ */
