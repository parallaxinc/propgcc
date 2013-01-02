''*********************************************
''* Propeller II Second-Stage Loader          *
''*  (c) 2012 David Betz                      *
''* Based on code by Chip Gracey from the     *
''* Propeller II ROM Loader                   *
''*  (c) 2012 Parallax                        *
''* MIT License                               *
''*********************************************

CON

  BASE = $0e80
  CLOCK_FREQ = 60000000
  BAUD = 115200
  SERIAL_TX = 90        ' must be in the port c
  SERIAL_RX = 91

  ' character codes
  SOH = $01             ' start of a packet
  ACK = $06
  NAK = $15

  #0
  STATE_SOH
  STATE_LEN_HI
  STATE_LEN_LO
  STATE_CHK
  STATE_DATA
  STATE_CRC_HI
  STATE_CRC_LO
  
DAT

                        org
                        
                        jmp     #init
                        
' these values will be patched by the loader based on user options
freq                    long    CLOCK_FREQ
period                  long    CLOCK_FREQ / BAUD
cogimage                long    BASE
stacktop                long    $20000
                        
init                    reps    #$1F6-reserves,#1       'clear reserves
                        setinda reserves
                        mov     inda++,#0
        
                        setp    tx_pin
                        mov     dirc,dirc_mask          'make tx pin an output
        
                        jmptask #rx_task,#%0010         'enable serial receiver task
                        settask #%%1010

                        mov     rcv_state, #STATE_SOH   'initialize the packet receive state
                        mov     rcv_base, base_addr     'initialize the start of packet
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

do_chk                  and     rcv_chk, #$ff
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
                        mov     rcv_base, rcv_ptr     'move to the next packet
                        mov     x, #ACK
                        call    #tx
                        tjz     rcv_length, #start    'start the program on a zero-length packet
                        mov     rcv_state, #STATE_SOH
                        jmp     #next_packet          'byte done, receive next byte

send_nak                mov     rcv_ptr, rcv_base     'reset to the start of the packet
                        mov     x, #NAK
                        jmp     #ack_nak              'look for the next packet

updcrc                  mov     t1, crc
                        test    t1, #$100 wz
                        shr     t1, #9
                        add     t1, #crctab
                        movs    :load, t1
                        shl     crc, #8
:load                   mov     t1, 0-0
              if_nz     shr     t1, #16
                        xor     crc, t1
                        xor     crc, x
                        and     crc, word_mask
updcrc_ret              ret

start                   coginit cogimage, stacktop
      'relaunch cog0 with loaded program

'
'
' Transmit chr (x)
'
tx                      shl     x,#1                    'insert start bit
                        setb    x,#9                    'set stop bit

                        getcnt  w                       'get initial time

:loop                   add     w,period                'add bit period to time
                        passcnt w                       'loop until bit period elapsed
                        shr     x,#1            wc      'get next bit into c
                        setpc   tx_pin                  'write c to tx pin
                        tjnz    x,#:loop                'loop until bits done

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
rx_check                or      rx_tail,#$80            'if start or rollover, reset tail

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
                if_z    setspb  #$80

                        mov     rx_bits,#9              'ready for 8 data bits + 1 stop bit

                        neg     rx_time,period          'get -0.5 period
                        sar     rx_time,#1

                        jp      rx_pin,#$               'wait for start bit

                        subcnt  rx_time                 'get time + 0.5 period for initial 1.5 period delay

:bit                    rcr     rx_data,#1              'rotate c into byte
                        add     rx_time,period          'add 1 period
                        passcnt rx_time                 'wait for center of next bit
                        getp    rx_pin          wc      'read rx pin into c
                        djnz    rx_bits,#:bit           'loop until 8 data bits + 1 stop bit received

                        shr     rx_data,#32-8           'align byte
                        pushb   rx_data                 'store byte at head, inc head

                        jmp     #rx_task                'wait for next byte


'*************
'* Constants *
'*************

rx_pin                  long    SERIAL_RX
tx_pin                  long    SERIAL_TX
dirc_mask               long    1 << (SERIAL_TX - 64)
base_addr               long    BASE
word_mask               long    $ffff

crctab
    word $0000,  $1021,  $2042,  $3063,  $4084,  $50a5,  $60c6,  $70e7
    word $8108,  $9129,  $a14a,  $b16b,  $c18c,  $d1ad,  $e1ce,  $f1ef
    word $1231,  $0210,  $3273,  $2252,  $52b5,  $4294,  $72f7,  $62d6
    word $9339,  $8318,  $b37b,  $a35a,  $d3bd,  $c39c,  $f3ff,  $e3de
    word $2462,  $3443,  $0420,  $1401,  $64e6,  $74c7,  $44a4,  $5485
    word $a56a,  $b54b,  $8528,  $9509,  $e5ee,  $f5cf,  $c5ac,  $d58d
    word $3653,  $2672,  $1611,  $0630,  $76d7,  $66f6,  $5695,  $46b4
    word $b75b,  $a77a,  $9719,  $8738,  $f7df,  $e7fe,  $d79d,  $c7bc
    word $48c4,  $58e5,  $6886,  $78a7,  $0840,  $1861,  $2802,  $3823
    word $c9cc,  $d9ed,  $e98e,  $f9af,  $8948,  $9969,  $a90a,  $b92b
    word $5af5,  $4ad4,  $7ab7,  $6a96,  $1a71,  $0a50,  $3a33,  $2a12
    word $dbfd,  $cbdc,  $fbbf,  $eb9e,  $9b79,  $8b58,  $bb3b,  $ab1a
    word $6ca6,  $7c87,  $4ce4,  $5cc5,  $2c22,  $3c03,  $0c60,  $1c41
    word $edae,  $fd8f,  $cdec,  $ddcd,  $ad2a,  $bd0b,  $8d68,  $9d49
    word $7e97,  $6eb6,  $5ed5,  $4ef4,  $3e13,  $2e32,  $1e51,  $0e70
    word $ff9f,  $efbe,  $dfdd,  $cffc,  $bf1b,  $af3a,  $9f59,  $8f78
    word $9188,  $81a9,  $b1ca,  $a1eb,  $d10c,  $c12d,  $f14e,  $e16f
    word $1080,  $00a1,  $30c2,  $20e3,  $5004,  $4025,  $7046,  $6067
    word $83b9,  $9398,  $a3fb,  $b3da,  $c33d,  $d31c,  $e37f,  $f35e
    word $02b1,  $1290,  $22f3,  $32d2,  $4235,  $5214,  $6277,  $7256
    word $b5ea,  $a5cb,  $95a8,  $8589,  $f56e,  $e54f,  $d52c,  $c50d
    word $34e2,  $24c3,  $14a0,  $0481,  $7466,  $6447,  $5424,  $4405
    word $a7db,  $b7fa,  $8799,  $97b8,  $e75f,  $f77e,  $c71d,  $d73c
    word $26d3,  $36f2,  $0691,  $16b0,  $6657,  $7676,  $4615,  $5634
    word $d94c,  $c96d,  $f90e,  $e92f,  $99c8,  $89e9,  $b98a,  $a9ab
    word $5844,  $4865,  $7806,  $6827,  $18c0,  $08e1,  $3882,  $28a3
    word $cb7d,  $db5c,  $eb3f,  $fb1e,  $8bf9,  $9bd8,  $abbb,  $bb9a
    word $4a75,  $5a54,  $6a37,  $7a16,  $0af1,  $1ad0,  $2ab3,  $3a92
    word $fd2e,  $ed0f,  $dd6c,  $cd4d,  $bdaa,  $ad8b,  $9de8,  $8dc9
    word $7c26,  $6c07,  $5c64,  $4c45,  $3ca2,  $2c83,  $1ce0,  $0cc1
    word $ef1f,  $ff3e,  $cf5d,  $df7c,  $af9b,  $bfba,  $8fd9,  $9ff8
    word $6e17,  $7e36,  $4e55,  $5e74,  $2e93,  $3eb2,  $0ed1,  $1ef0


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
rcv_base                res     1
rcv_ptr                 res     1
rcv_cnt                 res     1
crc                     res     1

                        fit     496
