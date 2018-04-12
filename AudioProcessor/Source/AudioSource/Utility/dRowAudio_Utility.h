/*
  ==============================================================================

  This file is part of the dRowAudio JUCE module
  Copyright 2004-13 by dRowAudio.

  ------------------------------------------------------------------------------

  dRowAudio is provided under the terms of The MIT License (MIT):

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
  SOFTWARE.

  ==============================================================================
*/

#ifndef __DROWAUDIO_UTILITY_H__
#define __DROWAUDIO_UTILITY_H__

#if JUCE_MSVC
    #pragma warning (disable: 4505)
#endif
#include "dRowAudio_Constants.h"
/** Converts an absolute value to decibels.
 */
forcedinline static double toDecibels (double absoluteValue)
{
    return 20.0 * log10 (absoluteValue);
}

/** Converts a value in decibels to an absolute value.
 */
forcedinline static double decibelsToAbsolute (double decibelsValue)
{
    return pow (10, (decibelsValue * 0.05));
}

/** Converts a time in seconds to minutes.
 */
forcedinline static double secondsToMins (double seconds)
{
    return seconds * oneOver60;
}

/** Converts a time in seconds to a number of samples for a given sample rate.
 */
forcedinline static int64 secondsToSamples (double timeSeconds, double sampleRate)
{
    return (int64) (timeSeconds * sampleRate);
}

/**	Reverses an array.
 */
template <class Type>
void reverseArray (Type* array, int length)
{
    Type swap;
	
    for (int a = 0; a < --length; a++)  //increment a and decrement b until they meet eachother
    {
        swap = array[a];                //put what's in a into swap space
        array[a] = array[length];       //put what's in b into a
        array[length] = swap;           //put what's in the swap (a) into b
    }
}

/**	Reverses two arrays at once.
	This will be quicker than calling reverseArray twice.
	The arrays must be the same length.
 */
template <class Type>
void reverseTwoArrays (Type* array1, Type* array2, int length)
{
    Type swap;
    
    for (int a = 0; a < --length; a++)  //increment a and decrement b until they meet eachother
    {
        swap = array1[a];               //put what's in a into swap space
        array1[a] = array1[length];     //put what's in b into a
        array1[length] = swap;          //put what's in the swap (a) into b

        swap = array2[a];               //put what's in a into swap space
        array2[a] = array2[length];     //put what's in b into a
        array2[length] = swap;          //put what's in the swap (a) into b
    }
}


#endif //__DROWAUDIO_UTILITY_H__