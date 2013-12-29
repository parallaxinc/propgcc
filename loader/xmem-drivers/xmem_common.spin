{
  External Memory Driver Common Code
  Copyright (c) 2013 by David Betz
  
  Based on code from Chip Gracey's Propeller II SDRAM Driver
  Copyright (c) 2013 by Chip Gracey

  TERMS OF USE: MIT License

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
}

PUB image
  return @init_xmem

DAT
        org   $0

init_xmem
        jmp     #init_continue
        
xmem_param1
        long    0
xmem_param2
        long    0
xmem_param3
        long    0
xmem_param4
        long    0
        
init_continue

        ' cmdbase is the base of an array of mailboxes
        mov     cmdbase, par

        ' initialize the read/write functions
        call    #init

        ' start the command loop
waitcmd
#ifdef RELEASE_PINS
        mov     dira, #0                ' release the pins for other SPI clients
#endif
:reset  mov     cmdptr, cmdbase
:loop
#ifdef REFRESH
        djnz    refresh_cnt, #:norefresh' check to see if its time to refresh
        call    #refresh_memory         ' if refresh timer expired, reload and refresh
:norefresh
#endif
        rdlong  t1, cmdptr wz
  if_z  jmp     #:next                  ' skip this mailbox if it's zero
        cmp     t1, #$8 wz              ' check for the end of list marker
  if_z  jmp     #:reset
        mov     hubaddr, t1             ' get the hub address
        andn    hubaddr, #$f
        mov     stsptr, cmdptr          ' get the external address and status pointer
        add     stsptr, #4
        rdlong  extaddr, stsptr         ' get the external address
        mov     t2, t1                  ' get the byte count
        and     t2, #7
        mov     count, #8
        shl     count, t2
#ifdef RELEASE_PINS
        mov     dira, pindir            ' setup the pins so we can use them
#endif
        test    t1, #$8 wz              ' check the write flag
  if_z  jmp     #:read                  ' do read if the flag is zero
        call    #write_bytes            ' do write if the flag is one
        jmp     #:done
:read   call    #read_bytes
:done
#ifdef RELEASE_PINS
        mov     dira, #0                ' release the pins for other SPI clients
#endif
        wrlong  t1, stsptr              ' return completion status
        wrlong  zero, cmdptr
:next   add     cmdptr, #8
        jmp     #:loop

' pointers to mailbox array
cmdbase long    0       ' base of the array of mailboxes
cmdptr  long    0       ' pointer to the current mailbox
stsptr  long    0       ' pointer to where to store the completion status

' input parameters to read_bytes and write_bytes
extaddr long    0       ' external address
hubaddr long    0       ' hub address
count   long    0

zero    long    0       ' zero constant
t1      long    0       ' temporary variable
t2      long    0       ' temporary variable
t3      long    0       ' temporary variable

'----------------------------------------------------------------------------------------------------
'
' init - initialize external memory
'
' on input:
'   xmem_param1 - xmem_param4 are initialization parameters filled in by the loader from the .cfg file
'
'----------------------------------------------------------------------------------------------------

'----------------------------------------------------------------------------------------------------
'
' read_bytes - read data from external memory
'
' on input:
'   extaddr is the external memory address to read
'   hubaddr is the hub memory address to write
'   count is the number of bytes to read
'
'----------------------------------------------------------------------------------------------------

'----------------------------------------------------------------------------------------------------
'
' write_bytes - write data to external memory
'
' on input:
'   extaddr is the external memory address to write
'   hubaddr is the hub memory address to read
'   count is the number of bytes to write
'
'----------------------------------------------------------------------------------------------------

'----------------------------------------------------------------------------------------------------
'
' refresh_memory - refresh external memory and reset refresh_cnt
'
' Note: only required if REFRESH is defined
'
'----------------------------------------------------------------------------------------------------
