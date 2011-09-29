//----------------------------------------------------------------------------------------------------------------------
// fft_bench.spin
//
// A simple FFT implmentation for use as a micro-controller benchmark. This is an in-place
// Radix-2 Decimation In Time FFT using fixed point arithmetic.
//
// When reimplementing this benchmark in other languages please honour the intention of
// this benchmark by following the algorithm as closely as possible. This version is based off
// of bech_fft.spin which is to be regarded as the "mother" of all versions of this benchmark
// in other languages. 
//
// This FFT was developed from the description by Douglas L. Jones at
// http://cnx.org/content/m12016/latest/.
// It is written as a direct implementation of the discussion and diagrams on that page
// with an emphasis on clarity and ease of understanding rather than speed.
//
// Michael Rychlik. 2011-02-27
//
// This file is released under the terms of the MIT license. See below.
//
// Credits:
//
//     A big thank you to Dave Hein for clearing up some issues during a great FFT debate on
//     the Parallax Inc Propller discussion forum:
//     http://forums.parallax.com/showthread.php?127306-Fourier-for-dummies-under-construction
//
// History:
//
// 2011-02-27    v1.0  Initial version.
// 2011-02-27    v1.0a Revert to separate twiddle factor tables (wx/wy) as
//                     overlapping tables were causing problems for zpugcc.
//
//----------------------------------------------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------------------------------------------
//#include <stdio.h>
#include "stdio.h"

#if defined(__propeller__)
#include "propeller.h"
#define int32_t int
#define int16_t short int
#else
#include <stdint.h>
#include <sys/time.h>
#endif


// Specify size of FFT buffer here with length and log base 2 of the length.
// N.B. Changing this will require changing the "twiddle factor" tables.
//     and may also require changing the fixed point format (if going bigger)
#define FFT_SIZE      1024
#define LOG2_FFT_SIZE 10

// cos and sin parts of the signal to be analysed
// Result is written back to here.
// Just write input sammles to bx and zero all by.
static int32_t bx[FFT_SIZE];
static int32_t by[FFT_SIZE];

static void fillInput();
static void decimate();
static void butterflies();
static void printSpectrum();
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
//#include <sys/time.h>
// Return a timestamp in microsecond resolution.
int time_ms()
{
#if defined(__propeller__)
    //printf("CLKFREQ: %lu\n", CLKFREQ);
    //printf("CNT %08x\n", CNT);
    return (CNT / (CLKFREQ / 1000));

#else
    struct timeval timeval;

    if (gettimeofday(&timeval, 0) == 0)
    {
        return ((timeval.tv_sec * 1000000) + timeval.tv_usec);
    
    }
    else
    {
        return(0);
    }
#endif
}
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
void fft_bench()
{
    int startTime, endTime;

    printf ("fft_bench v1.0\n");

    // Input some data
    fillInput();

    // Start benchmark timer
    startTime = time_ms();
    
    // Radix-2 Decimation In Time, the bit-reversal step.
    decimate();

    // The actual Fourier transform work.
    butterflies();

    // Stop benchmark timer
    endTime = time_ms();

    // Print resulting spectrum
    printSpectrum();

    printf ("1024 point bit-reversal and butterfly run time = %d ms\n", (endTime - startTime));
}
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
static int sqrti(int parm1)
{
     int parm2 = 0;
     int parm3 = 1 << 30;
     while (parm3)
     {
         parm2 |= parm3;
         if (parm2 <= parm1)
         {
             parm1 -= parm2;
             parm2 += parm3;
         }
         else
             parm2 -= parm3;
         parm2 >>= 1;
         parm3 >>= 2;
     }
     parm1 = parm2;
     return(parm1);
}
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
static void printSpectrum()
{
    int32_t f, real, imag,  magnitude;

    // Spectrum is available in first half of the buffers after FFT.
    printf("Freq.    Magnitude\n");
    for (f = 0; f <= FFT_SIZE / 2; f++)
    {
        // Frequency magnitde is square root of cos part sqaured plus sin part squared
        real = bx[f] / FFT_SIZE;
        imag = by[f] / FFT_SIZE;
        magnitude = sqrti ((real * real) + (imag * imag));
        if (magnitude > 0)
        {
            printf ("%08x %x\n", f, magnitude);
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
// For testing define 16 samples  of an input wave form here.
static int32_t input[] =  {4096, 3784, 2896, 1567, 0, -1567, -2896, -3784, -4096, -3784, -2896, -1567, 0, 1567, 2896, 3784};
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
// Fill buffer bx with samples of of an imput signal and clear by.
static void fillInput()
{
    int32_t k;

    for (k = 0; k <=FFT_SIZE - 1; k++)
    {
        // Two frequencies of the waveform defined in input
        bx[k]  = (input[(3*k) % 16] / 4);
        bx[k] += (input[(5*k) % 16] / 4);

        // The highest frequency
        if (k & 1)
          bx[k] += (4096 / 8);
        else
          bx[k] += (-4096 / 8);

        // A DC level
        bx[k] += (4096 / 8);

        // Clear Y array.
        by[k] = 0;
    }
}
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
#if defined(__propeller__) && defined(__GNUC__)
#define bitReverse(x, length) __builtin_propeller_rev(x, 32-(length))
#else
static unsigned int bitReverse(unsigned int x,  unsigned int length)
{
    x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
    x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
    x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
    x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
    x = (x >> 16) | (x << 16);
    return (x  >> (32 - length));
}
#endif
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
static void decimate()
{
    int32_t i, revi, tx1, ty1;

    // Radix-2 decimation in time.
    // Moves every sample of bx and by to a postion given by
    // reversing the bits of its original array index.
    for (i = 0; i <= FFT_SIZE - 1; i++)
    {
        revi = bitReverse (i, LOG2_FFT_SIZE);
        if (i < revi)
        {
            tx1 = bx[i];
            ty1 = by[i];

            bx[i] = bx[revi];
            by[i] = by[revi];

            bx[revi] = tx1;
            by[revi] = ty1;
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
static int16_t wx[512];
static int16_t wy[512];
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
static void butterflies()
{
    int32_t k1, k2, k3, a, b, c, d, flightSize, noFlights, b0, b1, wIndex, level, flight, butterfly, flightIndex, tx, ty;

    // Apply FFT butterflies to N complex samples in buffers bx and by, in time decimated order!
    // Resulting FFT is produced in bx and by in the correct order.
    flightSize = 1;
    noFlights = FFT_SIZE >> 1;

    // Loop though the decimation levels
    for (level = 0; level < LOG2_FFT_SIZE; level++)
    {
        flightIndex = 0;
        // Loop through each flight on a level.
        for (flight = 0; flight < noFlights; flight++)
        {
            wIndex = 0;
            // Loop through butterflies within a flight.
            for (butterfly = 0; butterfly < flightSize; butterfly++)
            {
                b0 = flightIndex + butterfly;
                b1 = b0 + flightSize;

                // At last...the butterfly.
                a = bx[b1];                 // Get X[b1]
                b = by[b1];

                c = wx[wIndex];             // Get W[wIndex]
                d = wy[wIndex];

                k1 = (a * (c + d)) >> 12;   // Somewhat optimized complex multiply
                k2 = (d * (a + b)) >> 12;   //     T = X[b1] * W[wIndex]
                k3 = (c * (b - a)) >> 12;
 
                tx = k1 - k2;
                ty = k1 + k3;

                k1 = bx[b0];
                k2 = by[b0];
                bx[b1] = k1 - tx;           // X[b1] = X[b0] * T
                by[b1] = k2 - ty;

                bx[b0] = k1 + tx;           // X[b0] = X[b0] * T
                by[b0] = k2 + ty;

                wIndex += noFlights;
            }
            flightIndex += (flightSize << 1);
        }
        flightSize <<= 1;
        noFlights >>= 1;
    }
}
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    fft_bench();
    return(0);
}
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
static int16_t wx[] = {
     4095,  4094,  4094,  4094,  4093,  4093,  4092,  4091,  4090,  4088,  4087,  4085,  4083,  4081,  4079,  4077, 
     4075,  4072,  4070,  4067,  4064,  4061,  4057,  4054,  4050,  4046,  4042,  4038,  4034,  4030,  4025,  4021, 
     4016,  4011,  4006,  4000,  3995,  3989,  3984,  3978,  3972,  3966,  3959,  3953,  3946,  3939,  3932,  3925, 
     3918,  3911,  3903,  3896,  3888,  3880,  3872,  3864,  3855,  3847,  3838,  3829,  3820,  3811,  3802,  3792, 
     3783,  3773,  3763,  3753,  3743,  3733,  3723,  3712,  3701,  3691,  3680,  3668,  3657,  3646,  3634,  3623, 
     3611,  3599,  3587,  3575,  3563,  3550,  3537,  3525,  3512,  3499,  3486,  3473,  3459,  3446,  3432,  3418, 
     3404,  3390,  3376,  3362,  3348,  3333,  3318,  3304,  3289,  3274,  3258,  3243,  3228,  3212,  3197,  3181, 
     3165,  3149,  3133,  3117,  3100,  3084,  3067,  3051,  3034,  3017,  3000,  2983,  2965,  2948,  2930,  2913, 
     2895,  2877,  2859,  2841,  2823,  2805,  2787,  2768,  2750,  2731,  2712,  2693,  2674,  2655,  2636,  2617, 
     2597,  2578,  2558,  2539,  2519,  2499,  2479,  2459,  2439,  2419,  2398,  2378,  2357,  2337,  2316,  2295, 
     2275,  2254,  2233,  2211,  2190,  2169,  2148,  2126,  2105,  2083,  2061,  2040,  2018,  1996,  1974,  1952, 
     1930,  1908,  1885,  1863,  1841,  1818,  1796,  1773,  1750,  1728,  1705,  1682,  1659,  1636,  1613,  1590, 
     1567,  1543,  1520,  1497,  1473,  1450,  1426,  1403,  1379,  1355,  1332,  1308,  1284,  1260,  1236,  1212, 
     1188,  1164,  1140,  1116,  1092,  1067,  1043,  1019,   994,   970,   946,   921,   897,   872,   848,   823, 
      798,   774,   749,   724,   700,   675,   650,   625,   600,   575,   551,   526,   501,   476,   451,   426, 
      401,   376,   351,   326,   301,   276,   251,   226,   200,   175,   150,   125,   100,    75,    50,    25, 
        0,   -25,   -50,   -75,  -100,  -125,  -150,  -175,  -200,  -226,  -251,  -276,  -301,  -326,  -351,  -376, 
     -401,  -426,  -451,  -476,  -501,  -526,  -551,  -576,  -600,  -625,  -650,  -675,  -700,  -724,  -749,  -774, 
     -798,  -823,  -848,  -872,  -897,  -921,  -946,  -970,  -995, -1019, -1043, -1067, -1092, -1116, -1140, -1164, 
    -1188, -1212, -1236, -1260, -1284, -1308, -1332, -1355, -1379, -1403, -1426, -1450, -1473, -1497, -1520, -1543, 
    -1567, -1590, -1613, -1636, -1659, -1682, -1705, -1728, -1750, -1773, -1796, -1818, -1841, -1863, -1885, -1908, 
    -1930, -1952, -1974, -1996, -2018, -2040, -2062, -2083, -2105, -2126, -2148, -2169, -2190, -2212, -2233, -2254, 
    -2275, -2295, -2316, -2337, -2357, -2378, -2398, -2419, -2439, -2459, -2479, -2499, -2519, -2539, -2558, -2578, 
    -2597, -2617, -2636, -2655, -2674, -2693, -2712, -2731, -2750, -2768, -2787, -2805, -2823, -2841, -2859, -2877, 
    -2895, -2913, -2930, -2948, -2965, -2983, -3000, -3017, -3034, -3051, -3067, -3084, -3100, -3117, -3133, -3149, 
    -3165, -3181, -3197, -3212, -3228, -3243, -3258, -3274, -3289, -3304, -3318, -3333, -3348, -3362, -3376, -3390, 
    -3404, -3418, -3432, -3446, -3459, -3473, -3486, -3499, -3512, -3525, -3537, -3550, -3563, -3575, -3587, -3599, 
    -3611, -3623, -3634, -3646, -3657, -3669, -3680, -3691, -3701, -3712, -3723, -3733, -3743, -3753, -3763, -3773, 
    -3783, -3792, -3802, -3811, -3820, -3829, -3838, -3847, -3855, -3864, -3872, -3880, -3888, -3896, -3903, -3911, 
    -3918, -3925, -3932, -3939, -3946, -3953, -3959, -3966, -3972, -3978, -3984, -3989, -3995, -4000, -4006, -4011, 
    -4016, -4021, -4025, -4030, -4034, -4038, -4043, -4046, -4050, -4054, -4057, -4061, -4064, -4067, -4070, -4072, 
    -4075, -4077, -4079, -4081, -4083, -4085, -4087, -4088, -4090, -4091, -4092, -4093, -4093, -4094, -4094, -4094, 
};

static int16_t wy[] = {
        0,   -25,   -50,   -75,  -100,  -125,  -150,  -175,  -200,  -226,  -251,  -276,  -301,  -326,  -351,  -376, 
     -401,  -426,  -451,  -476,  -501,  -526,  -551,  -576,  -600,  -625,  -650,  -675,  -700,  -724,  -749,  -774, 
     -798,  -823,  -848,  -872,  -897,  -921,  -946,  -970,  -995, -1019, -1043, -1067, -1092, -1116, -1140, -1164, 
    -1188, -1212, -1236, -1260, -1284, -1308, -1332, -1355, -1379, -1403, -1426, -1450, -1473, -1497, -1520, -1543, 
    -1567, -1590, -1613, -1636, -1659, -1682, -1705, -1728, -1750, -1773, -1796, -1818, -1841, -1863, -1885, -1908, 
    -1930, -1952, -1974, -1996, -2018, -2040, -2062, -2083, -2105, -2126, -2148, -2169, -2190, -2212, -2233, -2254, 
    -2275, -2295, -2316, -2337, -2357, -2378, -2398, -2419, -2439, -2459, -2479, -2499, -2519, -2539, -2558, -2578, 
    -2597, -2617, -2636, -2655, -2674, -2693, -2712, -2731, -2750, -2768, -2787, -2805, -2823, -2841, -2859, -2877, 
    -2895, -2913, -2930, -2948, -2965, -2983, -3000, -3017, -3034, -3051, -3067, -3084, -3100, -3117, -3133, -3149, 
    -3165, -3181, -3197, -3212, -3228, -3243, -3258, -3274, -3289, -3304, -3318, -3333, -3348, -3362, -3376, -3390, 
    -3404, -3418, -3432, -3446, -3459, -3473, -3486, -3499, -3512, -3525, -3537, -3550, -3563, -3575, -3587, -3599, 
    -3611, -3623, -3634, -3646, -3657, -3669, -3680, -3691, -3701, -3712, -3723, -3733, -3743, -3753, -3763, -3773, 
    -3783, -3792, -3802, -3811, -3820, -3829, -3838, -3847, -3855, -3864, -3872, -3880, -3888, -3896, -3903, -3911, 
    -3918, -3925, -3932, -3939, -3946, -3953, -3959, -3966, -3972, -3978, -3984, -3989, -3995, -4000, -4006, -4011, 
    -4016, -4021, -4025, -4030, -4034, -4038, -4043, -4046, -4050, -4054, -4057, -4061, -4064, -4067, -4070, -4072, 
    -4075, -4077, -4079, -4081, -4083, -4085, -4087, -4088, -4090, -4091, -4092, -4093, -4093, -4094, -4094, -4094, 
    -4094, -4094, -4094, -4094, -4093, -4093, -4092, -4091, -4090, -4088, -4087, -4085, -4083, -4081, -4079, -4077, 
    -4075, -4072, -4070, -4067, -4064, -4061, -4057, -4054, -4050, -4046, -4042, -4038, -4034, -4030, -4025, -4021, 
    -4016, -4011, -4006, -4000, -3995, -3989, -3984, -3978, -3972, -3966, -3959, -3953, -3946, -3939, -3932, -3925, 
    -3918, -3911, -3903, -3896, -3888, -3880, -3872, -3863, -3855, -3847, -3838, -3829, -3820, -3811, -3802, -3792, 
    -3783, -3773, -3763, -3753, -3743, -3733, -3723, -3712, -3701, -3691, -3680, -3668, -3657, -3646, -3634, -3623, 
    -3611, -3599, -3587, -3575, -3562, -3550, -3537, -3525, -3512, -3499, -3486, -3473, -3459, -3446, -3432, -3418, 
    -3404, -3390, -3376, -3362, -3347, -3333, -3318, -3304, -3289, -3274, -3258, -3243, -3228, -3212, -3197, -3181, 
    -3165, -3149, -3133, -3117, -3100, -3084, -3067, -3050, -3034, -3017, -3000, -2983, -2965, -2948, -2930, -2913, 
    -2895, -2877, -2859, -2841, -2823, -2805, -2787, -2768, -2749, -2731, -2712, -2693, -2674, -2655, -2636, -2617, 
    -2597, -2578, -2558, -2539, -2519, -2499, -2479, -2459, -2439, -2419, -2398, -2378, -2357, -2337, -2316, -2295, 
    -2275, -2254, -2233, -2211, -2190, -2169, -2148, -2126, -2105, -2083, -2061, -2040, -2018, -1996, -1974, -1952, 
    -1930, -1908, -1885, -1863, -1841, -1818, -1796, -1773, -1750, -1728, -1705, -1682, -1659, -1636, -1613, -1590, 
    -1567, -1543, -1520, -1497, -1473, -1450, -1426, -1403, -1379, -1355, -1332, -1308, -1284, -1260, -1236, -1212, 
    -1188, -1164, -1140, -1116, -1092, -1067, -1043, -1019,  -994,  -970,  -946,  -921,  -897,  -872,  -848,  -823, 
    -798,   -774,  -749,  -724,  -700,  -675,  -650,  -625,  -600,  -575,  -551,  -526,  -501,  -476,  -451,  -426, 
    -401,   -376,  -351,  -326,  -301,  -276,  -251,  -225,  -200,  -175,  -150,  -125,  -100,   -75,   -50,   -25
};
//----------------------------------------------------------------------------------------------------------------------
 
//----------------------------------------------------------------------------------------------------------------------
//    This file is distributed under the terms of the The MIT License as follows:
//
//    Copyright (c) 2011 Michael Rychlik
//
//    Permission is hereby granted, free of charge, to any person obtaining a copy
//    of this software and associated documentation files (the "Software"), to deal
//    in the Software without restriction, including without limitation the rights
//    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//    copies of the Software, and to permit persons to whom the Software is
//    furnished to do so, subject to the following conditions:
//
//    The above copyright notice and this permission notice shall be included in
//    all copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//    THE SOFTWARE.
//----------------------------------------------------------------------------------------------------------------------




