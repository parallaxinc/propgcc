{{
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// VGA64 Bitmap Engine
//
// Author: Kwabena W. Agyeman
// Updated: 7/15/2010
// Designed For: P8X32A
// Version: 1.1
//
// Copyright (c) 2010 Kwabena W. Agyeman
// See end of file for terms of use.
//
// Update History:
//
// v1.0 - Original release - 9/28/2009.
// v1.1 - Merged and rewrote code and added more features - 7/15/2010.
//
// For each included copy of this object only one spin interpreter should access it at a time.
//
// Nyamekye,
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Video Circuit:
//
//     0   1   2   3 Pin Group
//
//                     240OHM
// Pin 0,  8, 16, 24 ----R-------- Vertical Sync
//
//                     240OHM
// Pin 1,  9, 17, 25 ----R-------- Horizontal Sync
//
//                     470OHM
// Pin 2, 10, 18, 26 ----R-------- Blue Video
//                            |
//                     240OHM |
// Pin 3, 11, 19, 27 ----R-----
//
//                     470OHM
// Pin 4, 12, 20, 28 ----R-------- Green Video
//                            |
//                     240OHM |
// Pin 5, 13, 21, 29 ----R-----
//
//                     470OHM
// Pin 6, 14, 22, 30 ----R-------- Red Video
//                            |
//                     240OHM |
// Pin 7, 15, 23, 31 ----R-----
//
//                            5V
//                            |
//                            --- 5V
//
//                            --- Vertical Sync Ground
//                            |
//                           GND
//
//                            --- Hoirzontal Sync Ground
//                            |
//                           GND
//
//                            --- Blue Return
//                            |
//                           GND
//
//                            --- Green Return
//                            |
//                           GND
//
//                            --- Red Return
//                            |
//                           GND
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}}

CON

  #$FC, Light_Grey, #$A8, Grey, #$54, Dark_Grey
  #$C0, Light_Red, #$80, Red, #$40, Dark_Red
  #$30, Light_Green, #$20, Green, #$10, Dark_Green
  #$0C, Light_Blue, #$08, Blue, #$04, Dark_Blue
  #$F0, Light_Orange, #$A0, Orange, #$50, Dark_Orange
  #$CC, Light_Purple, #$88, Purple, #$44, Dark_Purple
  #$3C, Light_Teal, #$28, Teal, #$14, Dark_Teal
  #$FF, White, #$00, Black

  bitsPerPixel = 1
  horizontalScaling = 1
  horizontalPixels = 640

PUB BMPEngineStart(pinGroup, verticalResolution, newDisplayPointer, colortable, pixelColors, syncIndicator) '' 11 Stack Longs

'' ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
'' // Starts up the BMP driver running on a cog.
'' //
'' // Returns true on success and false on failure.
'' //
'' // PinGroup - Pin group to use to drive the video circuit. Between 0 and 3.
'' // ColorMode - Color mode to use for the whole screen. Between 1 bit per pixel or 2 bits per pixel.
'' // HorizontalResolution - The driver will force this value to be a factor of 640 and divisible by 16 or 32. 16/32 to 640.
'' // VerticalResolution - The driver will force this value to be a factor of 480. 1 to 480.
'' // NewDisplayPointer - The address of the new display buffer to be displayed after the vertical refresh.
'' ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    directionState := ($FF << (8 * pinGroup))
    videoState := ($20_00_00_FF | (pinGroup << 9) | ((bitsPerPixel) << 28))

    pinGroup := constant((25_175_000 + 1_600) / 4)
    frequencyState := 1

    repeat 32
      pinGroup <<= 1
      frequencyState <-= 1
      if(pinGroup => clkfreq)
        pinGroup -= clkfreq
        frequencyState += 1

    verticalScaling := 480 / verticalResolution
    verticalPixels := 480 / verticalScaling

    visibleScale := ((horizontalScaling << 12) + ((constant(640 * 32) >> bitsPerPixel) / horizontalPixels))
    invisibleScale := (((8 << bitsPerPixel) << 12) + 160)

    horizontalLongs := (horizontalPixels / (32 >> bitsPerPixel))

    buffer1 := newDisplayPointer
    colortable1 := colortable
    pixelColorsAddress := pixelColors
    syncIndicatorAddress := syncIndicator

    cognew(@initialization, 0)

DAT

' /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
'                       BMP Driver
' /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

                        org     0

' //////////////////////Initialization/////////////////////////////////////////////////////////////////////////////////////////

initialization          jmp     #skipover

' //////////////////////Configuration Settings/////////////////////////////////////////////////////////////////////////////////
directionState          long    0
videoState              long    0
frequencyState          long    0
verticalScaling         long    0
verticalPixels          long    0
visibleScale            long    0
invisibleScale          long    0
horizontalLongs         long    0
colortable1             long    0
buffer1                 long    0
pixelColorsAddress      long    0
syncIndicatorAddress    long    0


skipover
                        mov     vcfg,                 videoState                    ' Setup video hardware.
                        mov     frqa,                 frequencyState                '
                        movi    ctra,                 #%0_00001_101

' /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
'                       Active Video
' /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

loop                    mov     buffer, buffer1
                        mov     tilesCounter,         verticalPixels

tilesDisplay            mov     tileCounter,          verticalScaling               ' Set/Reset tile fill counter.
                        mov     tempxxx,              colortable1

tileDisplay             mov     vscl,                 visibleScale                  ' Set/Reset the video scale.
                        mov     counter,              horizontalLongs

' //////////////////////Visible Video//////////////////////////////////////////////////////////////////////////////////////////

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

                        rdbyte  AddressPtr,          buffer                        ' Download new pixels
                        shl     AddressPtr,         #6
                        add     AddressPtr,         tempxxx '
                        rdlong  tempyyy,              AddressPtr
                        waitvid screenColors,         tempyyy 'AddressPtr         ' Update display scanline.
                        add     buffer,               #1

' //////////////////////Invisible Video////////////////////////////////////////////////////////////////////////////////////////

                        mov     vscl,                 invisibleScale                ' Set/Reset the video scale.

                        waitvid HSyncColors,          syncPixels                    ' Horizontal Sync.

' //////////////////////Repeat/////////////////////////////////////////////////////////////////////////////////////////////////
                        add     tempxxx, #4 '
                        'sub     buffer,               horizontalLongs               ' Repeat.
                        sub     buffer,               horizontalLongs               ' Repeat.
                        djnz    tileCounter,          #tileDisplay                  '

                        'add    buffer,               horizontalLoops               ' Repeat.
                        'add     buffer,               horizontalLongs               ' Repeat.
                        add     buffer,               horizontalLongs               ' Repeat.
                        djnz    tilesCounter,         #tilesDisplay                 '

' /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
'                       Inactive Video
' /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

                        rdlong  screenColors,         pixelColorsAddress            ' Get new screen colors.
                        or      screenColors,         HVSyncColors                  '

' //////////////////////Update Indicator///////////////////////////////////////////////////////////////////////////////////////

                        add     refreshCounter,       #1                            ' Update sync indicator.
                        wrbyte  refreshCounter,       syncIndicatorAddress          '

' //////////////////////Front Porch////////////////////////////////////////////////////////////////////////////////////////////

                        mov     counter,              #11                           ' Set loop counter.

frontPorch              mov     vscl,                 blankPixels                   ' Invisible lines.
                        waitvid HSyncColors,          #0                            '

                        mov     vscl,                 invisibleScale                ' Horizontal Sync.
                        waitvid HSyncColors,          syncPixels                    '

                        djnz    counter,              #frontPorch                   ' Repeat # times.

' //////////////////////Vertical Sync//////////////////////////////////////////////////////////////////////////////////////////

                        mov     counter,              #(2 + 2)                      ' Set loop counter.

verticalSync            mov     vscl,                 blankPixels                   ' Invisible lines.
                        waitvid VSyncColors,          #0                            '

                        mov     vscl,                 invisibleScale                ' Vertical Sync.
                        waitvid VSyncColors,          syncPixels                    '

                        djnz    counter,              #verticalSync                 ' Repeat # times.

' //////////////////////Back Porch/////////////////////////////////////////////////////////////////////////////////////////////

                        mov     counter,              #31                           ' Set loop counter.

backPorch               mov     vscl,                 blankPixels                   ' Invisible lines.
                        waitvid HSyncColors,          #0                            '

                        mov     vscl,                 invisibleScale                ' Horizontal Sync.
                        waitvid HSyncColors,          syncPixels                    '

                        djnz    counter,              #backPorch                    ' Repeat # times.

' //////////////////////Update Display Settings////////////////////////////////////////////////////////////////////////////////

                        or      dira,                 directionState                '

' //////////////////////Loop///////////////////////////////////////////////////////////////////////////////////////////////////

                        jmp     #loop                                               ' Loop.

' /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
'                       Data
' /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

blankPixels             long    640                                                 ' Blank scanline pixel length.
syncPixels              long    $00_00_3F_FC                                        ' F-porch, h-sync, and b-porch.
HSyncColors             long    $01_03_01_03                                        ' Horizontal sync color mask.
VSyncColors             long    $00_02_00_02                                        ' Vertical sync color mask.
HVSyncColors            long    $03_03_03_03                                        ' Horizontal and vertical sync colors.


' //////////////////////Run Time Variables/////////////////////////////////////////////////////////////////////////////////////

tempxxx                 res     1
tempyyy                 res     1
temp1                   res     1

counter                 res     1
buffer                  res     1

tileCounter             res     1
tilesCounter            res     1

AddressPtr              res     1
screenColors            res     1

refreshCounter          res     1
displayCounter          res     1

' /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

                        fit     496

{{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                  TERMS OF USE: MIT License
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
// Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}}