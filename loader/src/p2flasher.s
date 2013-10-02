''*********************************************
''* Propeller II Second-Stage Loader          *
''*  (c) 2012 David Betz                      *
''* Based on code by Chip Gracey from the     *
''* Propeller II ROM Loader                   *
''*  (c) 2012 Parallax                        *
''* MIT License                               *
''*********************************************

  BASE = 0x0e80
  CLOCK_FREQ = 160000000
  BAUD = 115200
  
  
  ' must be in the port c
  FLASH_DO = 86         ' SPI flash pins
  FLASH_DI = 87
  FLASH_CLK = 88
  FLASH_CS = 89
  SERIAL_TX = 90        ' serial pins
  SERIAL_RX = 91

  ' character codes
  SOH = 0x01             ' start of a packet
  ACK = 0x06
  NAK = 0x15

  STATE_SOH = 0
  STATE_LEN_HI = 1
  STATE_LEN_LO = 2
  STATE_CHK = 3
  STATE_DATA = 4
  STATE_CRC_HI = 5
  STATE_CRC_LO = 6
  
  dirc = 0x1fe
  
  PAGE_SIZE = 4096
  
                        .pasm
                        org     0
                        
                        jmp     #init
                        
' these values will be patched by the loader based on user options
period                  long    CLOCK_FREQ / BAUD
cogimage                long    BASE
stacktop                long    0x20000
clkmode                 long    0xff '%1111_11_11       'PLL 16X
                        
init                    clkset	clkmode
                        repd    reserves_cnt_m1, #1     'clear reserves
                        setinda #reserves
                        nop
                        nop
                        mov     inda++,#0
        
                        setp    tx_pin
                        setp    #FLASH_CS
                        mov     dirc,dirc_mask          'make tx pin an output
        
                        jmptask #rx_task,#%0010         'enable serial receiver task
                        settask #0x44 '%%1010

                        mov     rcv_state, #STATE_SOH   'initialize the packet receive state
                        mov     rcv_ptr, base_addr      'initialize the hub memory pointer

                        'there must be space in the transmit buffer for this character
                        mov     x, #ACK                 'tell the sender we're ready
                        
ack_nak                 call    #tx
next_packet             mov     rcv_state, #STATE_SOH
receive                 call    #rx
                        mov     t1, rcv_state
                        add     t1, #dispatch
                        jmp     t1

dispatch                jmp     #do_soh
                        jmp     #do_len_hi
                        jmp     #do_len_lo
                        jmp     #do_chk
                        jmp     #do_data
                        jmp     #do_crc_hi
                        jmp     #do_crc_lo

do_soh                  cmp     x, #SOH wz
              if_z      mov     rcv_state, #STATE_LEN_HI
                        jmp     #receive              'byte done, receive next byte

do_len_hi               mov     rcv_length, x
                        shl     rcv_length, #8
                        mov     rcv_chk, x
                        mov     rcv_state, #STATE_LEN_LO
                        jmp     #receive              'byte done, receive next byte

do_len_lo               or      rcv_length, x
                        add     rcv_chk, x
                        mov     rcv_state, #STATE_CHK
                        jmp     #receive              'byte done, receive next byte

do_chk                  and     rcv_chk, #0xff
                        cmp     x, rcv_chk wz
              if_nz     jmp     #send_nak
                        mov     crc, #0
                        mov     rcv_cnt, rcv_length wz
              if_z      mov     rcv_state, #STATE_CRC_HI
              if_nz     mov     rcv_state, #STATE_DATA
                        jmp     #receive              'byte done, receive next byte

do_data                 call    #updcrc               'update the crc
                        wrbyte  x, rcv_ptr
                        add     rcv_ptr, #1
                        djnz    rcv_cnt, #receive
                        mov     rcv_state, #STATE_CRC_HI
                        jmp     #receive              'byte done, receive next byte

do_crc_hi               call    #updcrc               'update the crc
                        mov     rcv_state, #STATE_CRC_LO
                        jmp     #receive              'byte done, receive next byte

do_crc_lo               call    #updcrc               'update the crc
                        cmp     crc, #0 wz            'check the crc
              if_nz     jmp     #send_nak
                        tjz     rcv_length, #skip_data
                        mov     hubaddr, base_addr
                        mov     count, rcv_length
                        call    #write_block
                        mov     rcv_ptr, base_addr    'move to the next packet
skip_data               mov     x, #ACK
                        call    #tx
                        tjz     rcv_length, #start    'start the program on a zero-length packet
                        mov     rcv_state, #STATE_SOH
                        jmp     #next_packet          'byte done, receive next byte

send_nak                mov     rcv_ptr, base_addr    'reset to the start of the packet
                        mov     x, #NAK
                        jmp     #ack_nak              'look for the next packet

updcrc                  mov     t1, crc
                        test    t1, #0x100 wz
                        shr     t1, #9
                        add     t1, #crctab
                        movs    _load, t1
                        shl     crc, #8
_load                   mov     t1, 0-0
              if_nz     shr     t1, #16
                        xor     crc, t1
                        xor     crc, x
                        and     crc, word_mask
updcrc_ret              ret

start                   mov		t1, #0x100					'hardware reset
						clkset	t1
						coginit monitor_pgm, monitor_ptr    'relaunch cog0 with shut down or monitor
monitor_pgm             long    $70C                        'monitor program address
monitor_ptr             long    SERIAL_TX<<9 + SERIAL_RX    'monitor parameter (conveys pins)

' write_block
'   parameters:
'     vmaddr contains the target flash address
'     hubaddr contains the hup address of the data to write
'     count contains the number of bytes to write
'
wloop   test    vmaddr, #$ff wz
  if_nz jmp     #wdata
        setp    #FLASH_CS       ' release
        call    #wait_until_done
write_block
        test    vmaddr, pagemask wz
  if_z  call    #erase_4k_block
        call    #write_enable
        mov     cmd, vmaddr
        or      cmd, fprogram
        mov     bytes, #4
        call    #start_spi_cmd
wdata   rdbyte  data, hubaddr
        call    #spiSendByte
        add     hubaddr, #1
        add     vmaddr, #1
        djnz    count, #wloop
        setp    #FLASH_CS       ' release
        call    #wait_until_done
write_block_ret
        ret

erase_4k_block
        call    #write_enable
        mov     cmd, vmaddr
        or      cmd, ferase4kblk
        mov     bytes, #4
        call    #start_spi_cmd
        setp    #FLASH_CS       ' release
        call    #wait_until_done
erase_4k_block_ret
        ret

write_enable
        mov     cmd, fwrenable
        call    #start_spi_cmd_1
        setp    #FLASH_CS       ' release
write_enable_ret
        ret

wait_until_done
        mov     cmd, frdstatus
        call    #start_spi_cmd_1
_wait   call    #spiRecvByte
        test    data, #1 wz
  if_nz jmp     #_wait
        setp    #FLASH_CS       ' release
wait_until_done_ret
        ret

start_spi_cmd_1
        mov     bytes, #1
start_spi_cmd
        clrp    #FLASH_CS       ' select
_loop   rol     cmd, #8
        mov     data, cmd
        call    #spiSendByte
        djnz    bytes, #_loop
start_spi_cmd_1_ret
start_spi_cmd_ret
        ret

spiSendByte
        shl     data, #24
        mov     bits, #8
spiSend rol     data, #1 wc
        setpc   #FLASH_DI
        setp    #FLASH_CLK
        clrp    #FLASH_CLK
        djnz    bits, #spiSend
        setp    #FLASH_DI
spiSendByte_ret
        ret

spiRecvByte
        mov     bits, #8
spiRecv mov     data, #0
_rloop  setp    #FLASH_CLK
        getp    #FLASH_DO wc
        rcl     data, #1
        clrp    #FLASH_CLK
        djnz    bits, #_rloop
spiRecvByte_ret
        ret

' input parameters to BREAD and BWRITE
vmaddr      long    0       ' virtual address
hubaddr     long    0       ' hub memory address to read from or write to
count       long    0

' variables used by the spi send/receive functions
pagemask    long    PAGE_SIZE - 1
cmd         long    0
bytes       long    0
bits        long    0
data        long    0

' spi commands
fprogram    long    $02000000       ' flash program byte/page
ferase4kblk long    $20000000       ' flash erase a 4k block
frdstatus   long    $05000000       ' flash read status
fwrenable   long    $06000000       ' flash write enable

'
'
' Transmit chr (x)
'
tx                      shl     x,#1                    'insert start bit
                        setb    x,#9                    'set stop bit

                        getcnt  w                       'get initial time

tx_loop                 add     w,period                'add bit period to time
                        passcnt w                       'loop until bit period elapsed
                        shr     x,#1            wc      'get next bit into c
                        setpc   tx_pin                  'write c to tx pin
                        tjnz    x,#tx_loop              'loop until bits done

tx_ret                  ret

'
'
' Receive chr (x)
'
rx                      call    #rx_check               'wait for rx chr
               if_z     jmp     #rx

rx_ret                  ret
'
'
' Check receiver, z=0 if chr (x)
'
rx_check                or      rx_tail,#0x80            'if start or rollover, reset tail

                        getspb  rx_temp         wz      'if head uninitialized, z=1
              if_nz     cmp     rx_temp,rx_tail wz      'if head-tail mismatch, byte ready, z=0

              if_nz     getspa  rx_temp                 'preserve spa
              if_nz     setspa  rx_tail                 'get tail
              if_nz     popar   x                       'get byte at tail
              if_nz     getspa  rx_tail                 'update tail
              if_nz     setspa  rx_temp                 'restore spa

rx_check_ret            ret


'************************
'* Serial Receiver Task *
'************************

rx_task                 chkspb                  wz      'if start or rollover, reset head
                if_z    setspb  #0x80

                        mov     rx_bits,#9              'ready for 8 data bits + 1 stop bit

                        neg     rx_time,period          'get -0.5 period
                        sar     rx_time,#1

rx_wait                 jp      rx_pin,#rx_wait         'wait for start bit

                        subcnt  rx_time                 'get time + 0.5 period for initial 1.5 period delay

rx_bit                  rcr     rx_data,#1              'rotate c into byte
                        add     rx_time,period          'add 1 period
                        passcnt rx_time                 'wait for center of next bit
                        getp    rx_pin          wc      'read rx pin into c
                        djnz    rx_bits,#rx_bit         'loop until 8 data bits + 1 stop bit received

                        shr     rx_data,#32-8           'align byte
                        pushb   rx_data                 'store byte at head, inc head

                        jmp     #rx_task                'wait for next byte


'*************
'* Constants *
'*************

rx_pin                  long    SERIAL_RX
tx_pin                  long    SERIAL_TX
dirc_mask               long    (1<<(SERIAL_TX-64)) | (1<<(FLASH_DI-64)) | (1<<(FLASH_CLK-64)) | (1<<(FLASH_CS-64))
base_addr               long    BASE
word_mask               long    0xffff

crctab
    word 0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7
    word 0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef
    word 0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6
    word 0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de
    word 0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485
    word 0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d
    word 0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4
    word 0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc
    word 0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823
    word 0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b
    word 0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12
    word 0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a
    word 0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41
    word 0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49
    word 0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70
    word 0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78
    word 0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f
    word 0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067
    word 0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e
    word 0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256
    word 0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d
    word 0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405
    word 0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c
    word 0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634
    word 0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab
    word 0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3
    word 0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a
    word 0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92
    word 0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9
    word 0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1
    word 0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8
    word 0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0

reserves_cnt_m1         long    end_reserves-reserves-1


'*************
'* Variables *
'*************

reserves

w                       res     1                       'main task
x                       res     1
y                       res     1
z                       res     1

rx_tail                 res     1                       'serial receiver task
rx_temp                 res     1
rx_time                 res     1
rx_data                 res     1
rx_bits                 res     1

t1                      res     1
t2                      res     1

rxmask                  res     1
rxdata                  res     1
rxbits                  res     1
rxcnt                   res     1
rxcode                  res     1

tx_head                 res     1
tx_tail                 res     1
txmask                  res     1
txbuff                  res     1
txdata                  res     1
txbits                  res     1
txcnt                   res     1
txcode                  res     1

rcv_state               res     1
rcv_length              res     1
rcv_chk                 res     1
rcv_ptr                 res     1
rcv_cnt                 res     1
crc                     res     1

end_reserves

                        fit     496
