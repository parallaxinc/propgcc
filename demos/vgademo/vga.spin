{{
############################################################################
# This object implements a 640x480 4-color VGA display driver.  It is based
# in Kwabena W. Agyeman's (Kye) VGA64 Bitmap Engine.  This driver uses a
# 40x30 byte array to select one of 256 4-color tiles.  Each tiles consists
# of a 16x16 array of 2-bit pixels arranged as 16 long values, where each
# long is a row of pixels in the tile.
#
# The are 12 longs that must be set up in the cog image prior to starting a
# cog.  The 12 longs start at the second long (4th byte) in the cog image.
# The 12 longs are defined as follows:
#
#   directionState       - Bit mask or'ed with the DIRA register
#   videoState           - Value written to the VCFG register
#   frequencyState       - Value written to the FREQA register
#   numTileLines         - 16
#   numTileVert          - 480 / 16 = 30
#   visibleScale         - Value written to VSCL for visible lines
#   invisibleScale       - Value written to VSCL for blank lines
#   horizontalLongs      - Number of horizontal longs. Equals 640/16 = 40
#   tilePtr1             - Pointer to the tiles
#   tileMap1             - Pointer to the tile map array
#   pixelColorsAddress   - Pointer to a long describing the 4 colors
#   syncIndicatorAddress - Pointer to byte incremented each vertical interval
#
# For more information see the VgaStart routine.
#
# Written by Dave Hein
# Copyright (c) 2012 Parallax, Inc.
# MIT Licensed
############################################################################
}}

DAT

'///////////////////////////////////////////////////////////////////////////
'                       640x480x2 Tile Driver
'///////////////////////////////////////////////////////////////////////////

                        org     0

'///////////////////////////////////////////////////////////////////////////

initialization          jmp     #skipover

'///////////////////////////////////////////////////////////////////////////
directionState          long    0
videoState              long    0
frequencyState          long    0
numTileLines            long    0
numTileVert             long    0
visibleScale            long    0
invisibleScale          long    0
horizontalLongs         long    0
tilePtr1                long    0
tileMap1                long    0
pixelColorsAddress      long    0
syncIndicatorAddress    long    0


skipover
                        mov     vcfg, videoState          ' Setup video hardware.
                        mov     frqa, frequencyState
                        movi    ctra, #%0_00001_101

'///////////////////////////////////////////////////////////////////////////
'                       Active Video
'///////////////////////////////////////////////////////////////////////////

frame_loop              mov     tileMap2, tileMap1
                        mov     tileCounter, numTileVert

tileLoop                mov     lineCounter, numTileLines ' Set/Reset tile fill counter.
                        mov     tilePtr2, tilePtr1

lineLoop                mov     vscl, visibleScale        ' Set/Reset the video scale.

'///////////////////////////////////////////////////////////////////////////
'                       Generate 640 Pixels (40 Tiles)
'///////////////////////////////////////////////////////////////////////////

                        ' Tile 0
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 1
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 2
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 3
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 4
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 5
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 6
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 7
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 8
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 9
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 10
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 11
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 12
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 13
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 14
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 15
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 16
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 17
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 18
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 19
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 20
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 21
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 22
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 23
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 24
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 25
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 26
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 27
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 28
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 29
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 30
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 31
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 32
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 33
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 34
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 35
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 36
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 37
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 38
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

                        ' Tile 39
                        rdbyte  pixelPtr, tileMap2        ' Download new pixels
                        shl     pixelPtr, #6
                        add     pixelPtr, tilePtr2
                        rdlong  pixelVals, pixelPtr
                        waitvid screenColors, pixelVals   ' Update scanline.
                        add     tileMap2, #1

'///////////////////////////////////////////////////////////////////////////

                        mov     vscl, invisibleScale      ' Set/Reset the video scale.

                        waitvid HSyncColors, syncPixels   ' Horizontal Sync.

'///////////////////////////////////////////////////////////////////////////
                        add     tilePtr2, #4
                        sub     tileMap2, horizontalLongs ' Repeat.
                        djnz    lineCounter, #lineLoop

                        add     tileMap2, horizontalLongs ' Repeat.
                        djnz    tileCounter, #tileLoop

'///////////////////////////////////////////////////////////////////////////
'                       Inactive Video
'///////////////////////////////////////////////////////////////////////////

                        rdlong  screenColors, pixelColorsAddress          ' Get new screen colors.
                        or      screenColors, HVSyncColors

'/////////////////////Update Indicator/////////////////////////////////////

                        add     refreshCounter, #1        ' Update sync indicator.
                        wrbyte  refreshCounter, syncIndicatorAddress

'/////////////////////Front Porch//////////////////////////////////////////

                        mov     counter, #11              ' Set loop counter.

frontPorch              mov     vscl, blankPixels         ' Invisible lines.
                        waitvid HSyncColors, #0 

                        mov     vscl, invisibleScale      ' Horizontal Sync.
                        waitvid HSyncColors, syncPixels 

                        djnz    counter, #frontPorch      ' Repeat # times.

'/////////////////////Vertical Sync////////////////////////////////////////

                        mov     counter, #(2 + 2)         ' Set loop counter.

verticalSync            mov     vscl, blankPixels         ' Invisible lines.
                        waitvid VSyncColors, #0 

                        mov     vscl, invisibleScale      ' Vertical Sync.
                        waitvid VSyncColors, syncPixels

                        djnz    counter, #verticalSync    ' Repeat # times.

'/////////////////////Back Porch///////////////////////////////////////////

                        mov     counter, #31              ' Set loop counter.

backPorch               mov     vscl, blankPixels         ' Invisible lines.
                        waitvid HSyncColors, #0

                        mov     vscl, invisibleScale      ' Horizontal Sync.
                        waitvid HSyncColors, syncPixels

                        djnz    counter, #backPorch       ' Repeat # times.

'/////////////////////Update Display Settings//////////////////////////////

                        or      dira, directionState

'/////////////////////Loop/////////////////////////////////////////////////

                        jmp     #frame_loop

'//////////////////////////////////////////////////////////////////////////
'                       Data
'//////////////////////////////////////////////////////////////////////////

blankPixels             long    640                       ' Blank scanline pixel length.
syncPixels              long    $00_00_3F_FC              ' F-porch, h-sync, and b-porch.
HSyncColors             long    $01_03_01_03              ' Horizontal sync color mask.
VSyncColors             long    $00_02_00_02              ' Vertical sync color mask.
HVSyncColors            long    $03_03_03_03              ' Horizontal and vertical sync colors.


'/////////////////////Run Time Variables///////////////////////////////////

tilePtr2                res     1
pixelVals               res     1

counter                 res     1
tileMap2                res     1

lineCounter             res     1
tileCounter             res     1

pixelPtr                res     1
screenColors            res     1

refreshCounter          res     1
displayCounter          res     1

'//////////////////////////////////////////////////////////////////////////

                        fit     496

pub Dummy
  return

{{
+--------------------------------------------------------------------
|  TERMS OF USE: MIT License
+--------------------------------------------------------------------
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
+------------------------------------------------------------------
}}

