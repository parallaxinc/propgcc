''*********************************************
''* Propeller II Second-Stage Loader          *
''*  (c) 2012 David Betz                      *
''* Based on code by Chip Gracey from the     *
''* Propeller II ROM Loader                   *
''*  (c) 2012 Parallax                        *
''* MIT License                               *
''*********************************************

  BASE = 0x0e80
  CLOCK_FREQ = 60000000
  BAUD = 115200
  
  
  ' must be in the port c
  FLASH_DO = 86         ' SPI flash pins
  FLASH_DI = 87
  FLASH_CLK = 88
  FLASH_CS = 89
  SERIAL_TX = 90        ' serial pins
  SERIAL_RX = 91

  CMD_LOAD = 1
  CMD_START = 2
  CMD_COGINIT = 3

  COG_IMAGE_SIZE = 2048
  HEADER_OFFSET = COG_IMAGE_SIZE

  dirc = 0x1fe
  
                        .pasm
                        org     0
                        
                        jmp     #init
                        
' these values will be patched by the loader based on user options
period                  long    CLOCK_FREQ / BAUD

init                    setp    #FLASH_CS

                        mov     vmaddr, header_off
                        
get_cmd                 setptra base_addr       'save hub buffer contents
                        rdlong  save0, ptra++
                        rdlong  save1, ptra++
                        rdlong  save2, ptra++
                        
                        mov     hubaddr, base_addr
                        mov     count, #12
                        call    #read_block
                        add     vmaddr, #12
                        
                        setptra base_addr       'load the command
                        rdlong  cmd0, ptra++
                        rdlong  cmd1, ptra++
                        rdlong  cmd2, ptra++
                        
                        setptra base_addr       'restore the hub buffer contents
                        wrlong  save0, ptra++
                        wrlong  save1, ptra++
                        wrlong  save2, ptra++
                        
cmd_handler             mov     t1, cmd0
                        and     t1, #0xff
                        
                        'check for CMD_LOAD
                        cmp     t1, #CMD_LOAD wz
                if_nz   jmp     #cmd_handler_1
                        mov     hubaddr, cmd1
                        mov     count, cmd2
                        call    #read_block
                        add     vmaddr, cmd2
                        jmp     #get_cmd
                        
                        'check for CMD_START
cmd_handler_1           cmp     t1, #CMD_START wz
                 if_nz  jmp     #cmd_handler_2
                        shr     cmd0, #8 wz
                        setcog  cmd0
                        coginit cmd1, cmd2           'launch a cog
                 if_nz  mov     t1, #0
                 if_nz  cogstop t1
                        ' should never reach here
                        
                        'check for CMD_COGINIT
cmd_handler_2           cmp     t1, #CMD_COGINIT wz
                 if_nz  jmp     #get_cmd
                        shr     cmd0, #8
                        setcog  cmd0
                        coginit cmd1, cmd2           'launch a cog
                        jmp     #get_cmd

t1                      long    0
save0                   long    0
save1                   long    0
save2                   long    0
cmd0                    long    0
cmd1                    long    0
cmd2                    long    0
base_addr               long    BASE
header_off              long    HEADER_OFFSET

/*

hex                     mov     cnt, #8
_hloop                  rol     data, #4
                        mov     x, data
                        and     x, #0x0f
                        cmp     x, #10 wz, wc
                if_b    add     x, #0x30
                if_ae   add     x, #0x61-10
                        call    #tx
                        djnz    cnt, #_hloop
hex_ret                 ret

crlf                    mov     x, #0x0d
                        call    #tx
                        mov     x, #0x0a
                        call    #tx
crlf_ret                ret

cnt                     long    0

' Transmit chr (x)                              ' 12/13 longs
'
tx              setp    #SERIAL_TX              ' do it each time to simplify
                shl     x,#1                    ' insert start bit
                setb    x,#9                    ' set stop bit
                getcnt  w                       ' get initial time
_xloop          add     w,period                ' add bit period to time
                passcnt w                       ' loop until bit period elapsed
                shr     x,#1            wc      ' get next bit into c
                setpc   #SERIAL_TX              ' write c to tx pin
                tjnz    x,#_xloop               ' loop until 10 bits done
tx_ret          ret

x               long    0
w               long    0

*/

read_block
        mov     cmd, vmaddr
        or      cmd, fread
        mov     bytes, #5
        call    #start_spi_cmd
        mov     ptr, hubaddr
_loop   call    #spiRecvByte
        wrbyte  data, ptr
        add     ptr, #1
        djnz    count, #_loop
        setp    #FLASH_CS       ' release
read_block_ret
        ret
        
start_spi_cmd_1
        mov     bytes, #1
start_spi_cmd
        clrp    #FLASH_CS       ' select
_cloop  rol     cmd, #8
        mov     data, cmd
        call    #spiSendByte
        djnz    bytes, #_cloop
start_spi_cmd_1_ret
start_spi_cmd_ret
        ret

spiSendByte
        shl     data, #24
        mov     bits, #8
_sloop  rol     data, #1 wc
        setpc   #FLASH_DI
        setp    #FLASH_CLK
        clrp    #FLASH_CLK
        djnz    bits, #_sloop
        setp    #FLASH_DI
spiSendByte_ret
        ret

spiRecvByte
        mov     bits, #8
        mov     data, #0
_rloop  setp    #FLASH_CLK
        getp    #FLASH_DO wc
        rcl     data, #1
        clrp    #FLASH_CLK
        djnz    bits, #_rloop
spiRecvByte_ret
        ret

' input parameters to read_block
vmaddr      long    0       ' virtual address
hubaddr     long    0       ' hub memory address to read from or write to
count       long    0

' variables used by the spi send/receive functions
cmd         long    0
ptr         long    0
bytes       long    0
bits        long    0
data        long    0

' spi commands
fread       long    $0b000000       ' flash read command

            fit     496
