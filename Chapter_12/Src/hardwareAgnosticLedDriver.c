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

#include "hardwareAgnosticLedDriver.h"
#include <stdlib.h>
#include <stdint.h>

/**
 * doLedStuff uses a hardware agnostic interface definition
 * (iLed) to operate on an LED.  The exact implementation
 * of iLed is flexible (it could be something as simple
 * as a GPIO line - or more complex (like a NeoPixel).
 * The LED could even be a remote IoT-based device - it makes
 * no difference, as long as it implements the functions
 * required by iLed
 */
void doLedStuff( iLed* LedPtr )
{
	__attribute__ ((unused)) volatile uint32_t dontCare = 0;

	if(LedPtr != NULL)
	{
		if(LedPtr->On != NULL)
		{
			LedPtr->On();
		}

		for(uint32_t i = 0; i < 9000000; i++)
		{
			dontCare = i % 4;
		}

		if( LedPtr->Off != NULL )
		{
			LedPtr->Off();
		}

		for(uint32_t i = 0; i < 9000000; i++)
		{
			dontCare = i % 4;
		}
	}
}
