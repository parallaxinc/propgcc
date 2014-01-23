{
  External memory interface
  by David Betz

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

CON

  ' external memory driver header structure
  HEADER2_JMP           = 0     ' jump around parameters
  HEADER2_CONFIG_1      = 1     ' driver specific configuration
  HEADER2_CONFIG_2      = 2
  HEADER2_CONFIG_3      = 3
  HEADER2_CONFIG_4      = 4

  ' external memory driver mailbox structure
  MBOX2_HUBADDR         = 0
  MBOX2_EXTADDR         = 1
  _MBOX2_SIZE           = 2
  
  ' marker for the end of the mailbox array
  MBOX2_END             = %1000

  ' external memory driver interface
  XMEM_WRITE            = %1000
  XMEM_SIZE_16          = %0001
  XMEM_SIZE_32          = %0010
  XMEM_SIZE_64          = %0011
  XMEM_SIZE_128         = %0100
  XMEM_SIZE_256         = %0101
  XMEM_SIZE_512         = %0110
  XMEM_SIZE_1024        = %0111

VAR
   long vm_mbox

PUB start2(code, mboxes, count)
    vm_mbox := mboxes ' use the first mailbox for now
    longfill(mboxes, 0, count * _MBOX2_SIZE)
    long[mboxes][count * _MBOX2_SIZE] := MBOX2_END
    return cognew(code, mboxes)

pub readBlock(madr, buf, size)
    long[vm_mbox][1] := madr
    long[vm_mbox][0] := buf | size
    repeat while long[vm_mbox][0] <> 0

' external memory driver interface
pub writeBlock(madr, buf, size)
    long[vm_mbox][1] := madr
    long[vm_mbox][0] := buf | XMEM_WRITE | size
    repeat while long[vm_mbox][0] <> 0

