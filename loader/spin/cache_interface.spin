{
  JCACHE interface
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

  INIT_MBOX             = 0     ' cache line mask should be returned here
  INIT_CACHE            = 1
  INIT_CONFIG_1         = 2     ' driver specific configuration
  INIT_CONFIG_2         = 3     ' driver specific configuration
  _INIT_SIZE            = 4

  ' mailbox offsets
  MBOX_CMD              = 0
  MBOX_ADDR             = 1
  _MBOX_SIZE            = 2

  ' cache access commands
  WRITE_CMD             = %10
  READ_CMD              = %11

  ' extended commands
  ERASE_CHIP_CMD        = %000_01
  ERASE_BLOCK_CMD       = %001_01
  WRITE_DATA_CMD        = %010_01
  SD_INIT_CMD           = %011_01
  SD_READ_CMD           = %100_01  ' only for C3 because of shared SPI pins
  SD_WRITE_CMD          = %101_01  ' only for C3 because of shared SPI pins

  CMD_MASK              = %11
  EXTEND_MASK           = %10   ' this bit must be zero for an extended command

VAR
   long vm_mbox
   long vm_linemask

PUB start(code, mbox, cache, config1, config2) | params[_INIT_SIZE]
    vm_mbox := mbox
    params[INIT_MBOX] := mbox
    params[INIT_CACHE] := cache
    params[INIT_CONFIG_1] := config1
    params[INIT_CONFIG_2] := config2
    long[vm_mbox] := $ffffffff
    cognew(code, @params)
    repeat while long[vm_mbox]
    vm_linemask := params[0]
    return vm_linemask

pub readLong(madr)
    long[vm_mbox][0] := (madr&!CMD_MASK) | READ_CMD
    repeat while long[vm_mbox][0]
    madr &= vm_linemask
    return long[long[vm_mbox][1]+madr]

pub writeLong(madr, val)
    long[vm_mbox][0] := (madr&!CMD_MASK) | WRITE_CMD
    repeat while long[vm_mbox][0]
    madr &= vm_linemask
    long[long[vm_mbox][1]+madr] := val

pub readByte(madr)
    long[vm_mbox][0] := (madr&!CMD_MASK) | READ_CMD
    repeat while long[vm_mbox][0]
    madr &= vm_linemask
    return byte[long[vm_mbox][1]+madr]

pub writeByte(madr, val)
    long[vm_mbox][0] := (madr&!CMD_MASK) | WRITE_CMD
    repeat while long[vm_mbox][0]
    madr &= vm_linemask
    byte[long[vm_mbox][1]+madr] := val

pub eraseFlashBlock(madr)
    long[vm_mbox][0] := ERASE_BLOCK_CMD | (madr << 8)
    repeat while long[vm_mbox][0] <> 0
    return long[vm_mbox][1]

pub writeFlash(madr, buf, count_) | pbuf, pcnt, paddr
    pbuf := buf
    pcnt := count_
    paddr := madr
    long[vm_mbox][0] := WRITE_DATA_CMD | (@pbuf << 8)
    repeat while long[vm_mbox][0] <> 0
    return long[vm_mbox][1]


