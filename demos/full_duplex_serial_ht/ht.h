//--------------------------------------------------------------------------------------------------
//
// ht.h
//
// Heater Threads.
//
// Macros for implementing coroutines or multi-threaded code.
// Even faster and more compact than Proto Threads.
//
// Uses the GCC labels as values extention.
//
// See end of file for example of use.
//
// Copyright (c) 2011 Michael Rychlik.
// MIT Licensed (see at end of file for exact terms)
//
// History:
//
// 2011-10-15    v1.0  Initial version.
//
//--------------------------------------------------------------------------------------------------
//
// Use to define a thread start location
// Saves having ugly labels in the thread code
#define HT_THREAD(name) name :

// Thread handles
#define HT_THREAD_T static _COGMEM volatile void* 
 
// Use concat to create labels for computed gotos 
#define concat_impl(x, y) x ## y
#define concat(x, y) concat_impl(x, y)

// Yield execution from the currently running thread to another.
// Uses the GCC labels as values extention. The task switch here should
// compile to a single Propeller JMPRET instruction when compiled as
// Propeller COG native code and optimized.
// Params:
//        thisThread A void* pointer into the currently running thread.
//        nextThread A void* pointer into the thread to swap to.
#define HT_YIELD(thisThread, nextThread) \
        thisThread = && concat(next_,__LINE__); goto *nextThread; concat(next_,__LINE__) :;

// Yield execution from the currently running thread to another
// while a given condition is true.
// Uses the GCC labels as values extention. The task switch here should
// compile to a single Propeller JMPRET instruction when compiled as
// COG native code and optimized.
// Params:
//        thisThread A void* pointer into the currently running thread.
//        nextThread A void* pointer into the thread to swap to.
//        cond       The condition.
#define HT_WAIT_WHILE(thisThread, nextThread, cond)            \
    while(cond)                                                   \
    {                                                             \
         HT_YIELD(thisThread, nextThread);                     \
    }        

// Yield execution from the currently running task to another task
// until a given conditin is true.
// Params:
//        thisTask A void* pointer into the currently running thread.
//        nextTask A void* pointer into the thread to swap to.
//        cond     The condition.
#define HT_WAIT_UNTIL(thisThread, nextThread, cond)             \
    HT_WAIT_WHILE (thisThread, nextThread, !(cond))

// A condition that evaluates to true when the current time
// is greater than the given time.
// Params:
//        t        The time after which the condition should be true.
#define HT_TIME_AFTER(t)  ((int)_CNT - t >= 0)


//--------------------------------------------------------------------------------------------------
// HT Threads Example. Two coroutines.
//
// N.B. All threads are contained within a single function.
/*

// Thread states
HT_THREAD_P ht_a;
HT_THREAD_P ht_b;

void some_function(...)
{
    // Set up coroutine pointers
    // Note use of GCC extention && (get the address of a label) 
    ht_a = &&aThread;
    ht_b = &&bThread;

    // First thread (one of two coroutines in this example)
    HT_THREAD(aThread)
    {
        // We loop forever here.
        while(1)
        {
            // Some code...
  
            // Wait until current time greater than 3000
            HT_WAIT_UNTIL(ht_a, ht_b, HT_TIME_AFTER(3000)); 

            // Some code...

            // Give other thread(s) some proceesing time
            HT_YIELD (ht_a, ht_b);
        }
    }

    // Second thread.
    // N.B. ht_a and ht_b thread params are in reverse order in this thread.
    HT_THREAD(bThread)
    {
        // We loop forever here.
        while(1)
        {
            // Wait while some condition is true.
	    HT_WAIT_WHILE (ht_b, ht_a, ((_INA >> rxPin) & 1));

	    // Give other thread(s) some proceesing time
            HT_YIELD (ht_b, ht_a);
        }
    }
}
*/
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// TERMS OF USE: MIT License
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files
// (the "Software"), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//--------------------------------------------------------------------------------------------------


