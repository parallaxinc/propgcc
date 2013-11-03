{
  External Memory Interface
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

  ' mailbox offsets
  MBOX_HUBADDR          = 0
  MBOX_EXTADDR          = 1
  _MBOX_SIZE            = 2

  ' transfer sizes
  TRANSFER_SIZE_16      = %0001
  TRANSFER_SIZE_32      = %0010
  TRANSFER_SIZE_64      = %0011
  TRANSFER_SIZE_128     = %0100
  TRANSFER_SIZE_256     = %0101
  TRANSFER_SIZE_512     = %0110
  TRANSFER_SIZE_1024    = %0111
  
  ' write mask
  WRITE_MASK            = %1000

VAR
   long xm_mbox

PUB start(code, mbox_count, mboxes)
    xm_mbox := mboxes   ' use the first mailbox
    longfill(mboxes, 0, _MBOX_SIZE * mbox_count)
    long[mboxes + _MBOX_SIZE * mbox_count] := $00000008
    cognew(code, @mboxes)
    
pub read(extaddr, hubaddr, buf, size)
    long[xm_mbox][MBOX_EXTADDR] := extaddr
    long[xm_mbox][MBOX_HUBADDR] := hubaddr | size
    repeat while long[xm_mbox][MBOX_HUBADDR]

pub write(extaddr, hubaddr, buf, size)
    long[xm_mbox][MBOX_EXTADDR] := extaddr
    long[xm_mbox][MBOX_HUBADDR] := hubaddr | WRITE_MASK | size
    repeat while long[xm_mbox][MBOX_HUBADDR]
