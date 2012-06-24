/**
 * @file keyboard.h - provides keyboard access via PASM
 *
 * Copyright (c) 2008, Steve Denson
 *
 * Based on features of and  and text included from Chip's keyboard.spin:
 * PS/2 Keyboard Driver v1.0.1      
 * Author: Chip Gracey              
 * Copyright (c) 2004 Parallax, Inc.
 *
 * ----------------------------------------------------------------------------
 *                        TERMS OF USE: MIT License
 * ----------------------------------------------------------------------------
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 * ----------------------------------------------------------------------------
 */

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

typedef volatile struct keybd_struct
{
  long  tail;        ///key buffer tail        read/write      (19 contiguous longs)
  long  head;        ///key buffer head        read-only
  long  present;     ///keyboard present       read-only
  long  states[8];   ///key states (256 bits)  read-only
  long  keys[8];     ///key buffer (16 words)  read-only       (also used to pass initial parameters)
} Keybd_st;

/**
 * Start keyboard driver - starts a cog
 *
 * @param dpin  = data signal on PS/2 jack
 * @param cpin  = clock signal on PS/2 jack
 *
 * @returns false if no cog available
 *
 * @note
 *    use 100-ohm resistors between pins and jack
 *    use 10K-ohm resistors to pull jack-side signals to VDD
 *    connect jack-power to 5V, jack-gnd to VSS
 * @note
 * all lock-keys will be enabled, NumLock will be initially 'on',
 * and auto-repeat will be set to 15cps with a delay of .5s
 */
int keybd_start(int dpin, int cpin);

/**
 * Like start, but allows you to specify lock settings and auto-repeat
 *
 * @param dpin  = data signal on PS/2 jack
 * @param cpin  = clock signal on PS/2 jack
 *
 * @param locks = lock setup
 *        bit 6 disallows shift-alphas (case set soley by CapsLock)
 *        bits 5..3 disallow toggle of NumLock/CapsLock/ScrollLock state
 *        bits 2..0 specify initial state of NumLock/CapsLock/ScrollLock
 *        (eg. %0_001_100 = disallow ScrollLock, NumLock initially 'on')
 *
 * @param autorep = auto-repeat setup
 *        bits 6..5 specify delay (0=.25s, 1=.5s, 2=.75s, 3=1s)
 *        bits 4..0 specify repeat rate (0=30cps..31=2cps)
 *        (eg %01_00000 = .5s delay, 30cps repeat)
 *
 * @returns false if no cog available
 */
int keybd_startx(int dpin, int cpin, int locks, int autorep);

/**
 * Stop keyboard driver - frees a cog
 */
void keybd_stop(void);

/**
 * Check if keyboard present - valid ~2s after start
 * @returns non-zero if present
 */
int keybd_present(void);
/**
 * Get key (never waits)
 * @returns key or 0 if buffer empty
 */
int keybd_key(void);

/**
 * Get next key (may wait for keypress)
 * @returns key
 */
int keybd_getkey(void);

/**
 * Clear buffer and get new key (always waits for keypress)
 * @returns key
 */
int keybd_newkey(void);

/**
 * Check if any key in buffer
 * @returns non-zero if key is in buffer
 */
int keybd_gotkey(void);

/**
 * Clear key buffer
 */
void keybd_clearkeys(void);

/**
 * Get the state of a particular key
 * @param key = key to check
 * @returns non-zero if key active
 */
int keybd_keystate(int key);

/**
 * @returns key or 0 if no key available ... does not block
 */
int kbhit(void);

/**
 * @fn getchar declared in stdio.h
 * @returns key after block for input
 */

/*
      Key Codes

      00..DF  = keypress and keystate
      E0..FF  = keystate only


      09      Tab
      0D      Enter
      20      Space
      21      !
      22      "
      23      #
      24      $
      25      %
      26      &
      27      '
      28      (
      29      )
      2A      *
      2B      +
      2C      ,
      2D      -
      2E      .
      2F      /
      30      0..9
      3A      :
      3B      ;
      3C      <
      3D      =
      3E      >
      3F      ?
      40      @       
      41..5A  A..Z
      5B      [
      5C      \
      5D      ]
      5E      ^
      5F      _
      60      `
      61..7A  a..z
      7B      {
      7C      |
      7D      }
      7E      ~

      80-BF   (future international character support)

      C0      Left Arrow
      C1      Right Arrow
      C2      Up Arrow
      C3      Down Arrow
      C4      Home
      C5      End
      C6      Page Up
      C7      Page Down
      C8      Backspace
      C9      Delete
      CA      Insert
      CB      Esc
      CC      Apps
      CD      Power
      CE      Sleep
      CF      Wakeup

      D0..DB  F1..F12
      DC      Print Screen
      DD      Scroll Lock
      DE      Caps Lock
      DF      Num Lock

      E0..E9  Keypad 0..9
      EA      Keypad .
      EB      Keypad Enter
      EC      Keypad +
      ED      Keypad -
      EE      Keypad *
      EF      Keypad /

      F0      Left Shift
      F1      Right Shift
      F2      Left Ctrl
      F3      Right Ctrl
      F4      Left Alt
      F5      Right Alt
      F6      Left Win
      F7      Right Win

      FD      Scroll Lock State
      FE      Caps Lock State
      FF      Num Lock State

      +100    if Shift
      +200    if Ctrl
      +400    if Alt
      +800    if Win

      eg. Ctrl-Alt-Delete = $6C9


 Note: Driver will buffer up to 15 keystrokes, then ignore overflow.

*/

#endif
