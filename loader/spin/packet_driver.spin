''*********************************************
''* Packet Driver                             *
''*  (C) 2011 David Betz                      *
''* Based on:                                 *
''*  Full-Duplex Serial Driver v1.1 Extended  *
''*   (C) 2006 Parallax, Inc.                 *
''*********************************************

CON

  ' status codes
  #0
  STATUS_PENDING  ' must be zero
  STATUS_OK
  STATUS_ERROR
  
  ' character codes
  SOH = $01       ' start of a packet
  ACK = $06
  NAK = $15

  #0
  STATE_SOH
  STATE_PKTN
  STATE_TYPE
  STATE_LEN_HI
  STATE_LEN_LO
  STATE_CHK
  STATE_DATA
  STATE_CRC_HI
  STATE_CRC_LO
  
  rxsize = 1024
  txsize = 16

  PKTMAXLEN = rxsize

VAR

  long mailbox[5]
  byte buffers[rxsize*2+txsize]

{
  init structure:
        long mailbox       '0: mailbox address
        long rx_pin        '1: receive pin
        long tx_pin        '2: transmit pin
        long rxtx_mode     '3: rx/tx mode
        long bit_ticks     '4: baud rate
        long rxlength      '5: size of receive packet data buffer
        long txlength      '6: size of transmit buffer (must be power of 2)
        long buffers       '7: rxlength*2+txlength size buffer

  mailbox structure:
        long type          '0: packet type
        long rxbuffer      '1: packet data buffer
        long rxlength      '2: packet data length
        long status        '3: command status
        long cog           '4: cog running driver
}

PUB start(rxpin, txpin, mode, baudrate)
  return startx(@mailbox, rxpin, txpin, mode, baudrate, rxsize, txsize, @buffers)

PUB startx(mbox, rxpin, txpin, mode, baudrate, rxsiz, txsiz, buffs) : okay

'' Start packet driver - starts a cog
'' returns false if no cog available
''
'' mode bit 0 = invert rx
'' mode bit 1 = invert tx
'' mode bit 2 = open-drain/source tx
'' mode bit 3 = ignore tx echo on rx

  ' stop the cog if it is already running
  stopx(mbox)

  ' compute the ticks per bit from the baudrate
  baudrate := clkfreq / baudrate

  ' start the driver cog
  okay := long[mbox][4] := cognew(@entry, @mbox) + 1

  ' if the cog started okay wait for it to finish initializing
  if okay
    repeat while mbox <> 0

PUB stop
  stopx(@mailbox)

PUB stopx(mbox)

  if long[mbox][4]
    cogstop(long[mbox][4]~ - 1)

PUB rcv_packet(ptype, pbuffer, plength)
  return rcv_packetx(@mailbox, ptype, pbuffer, plength)

PUB rcv_packetx(mbox, ptype, pbuffer, plength)

  repeat while long[mbox][3] == STATUS_PENDING

  if long[mbox][3] <> STATUS_OK
    long[mbox][3] := STATUS_PENDING
    return FALSE

  long[ptype] := long[mbox][0]
  long[pbuffer] := long[mbox][1]
  long[plength] := long[mbox][2]

  return TRUE

PUB release_packet
  release_packetx(@mailbox)

PUB release_packetx(mbox)
  long[mbox][3] := STATUS_PENDING

DAT

'***********************************
'* Assembly language serial driver *
'***********************************

                        org
'
'
' Entry
'
entry                   mov     t1, par              'get init structure address

                        rdlong  t2, t1               'get the mailbox address
                        mov     pkt_type_ptr, t2
                        add     t2, #4
                        mov     pkt_buffer_ptr, t2
                        add     t2, #4
                        mov     pkt_length_ptr, t2
                        add     t2, #4
                        mov     pkt_status_ptr, t2

                        add     t1, #4                'get rx_pin
                        rdlong  t2, t1
                        mov     rxmask, #1
                        shl     rxmask, t2

                        add     t1, #4                'get tx_pin
                        rdlong  t2, t1
                        mov     txmask, #1
                        shl     txmask, t2

                        add     t1, #4                'get rxtx_mode
                        rdlong  rxtxmode, t1

                        add     t1, #4                'get bit_ticks
                        rdlong  bitticks, t1

                        add     t1, #4                'get rxsize
                        rdlong  rcv_max, t1

                        add     t1, #4                'get txsize (power of 2)
                        rdlong  tx_buffer_mask, t1
                        sub     tx_buffer_mask, #1

                        add     t1, #4                'get buffer address
                        rdlong  rcv_buf1, t1
                        mov     rcv_buf2, rcv_buf1
                        add     rcv_buf2, rcv_max
                        mov     txbuff, rcv_buf2
                        add     txbuff, rcv_max

                        mov     t1, #STATUS_PENDING   'no packet available yet
                        wrlong  t1, pkt_status_ptr

                        mov     t1, #0                'signal end of initialization
                        wrlong  t1, par

                        test    rxtxmode,#%100  wz    'if_nz = open drain Tx
                        test    rxtxmode,#%010  wc    'if_c = inverted output
        if_z_ne_c       or      outa,txmask
        if_z            or      dira,txmask

                        mov     tx_head,#0            'clear the transmit buffer
                        mov     tx_tail,#0

                        mov     rxcode,#receive       'initialize ping-pong multitasking
                        mov     txcode,#transmit

                        mov     rcv_state, #STATE_SOH 'initialize the packet receive state

                        'there must be space in the transmit buffer for this character
                        mov     sndbyte, #ACK         'tell the sender we're ready
                        call    #send_byte

'
'
' Receive
'
receive                 jmpret  rxcode,txcode         'run a chunk of transmit code, then return

                        test    rxtxmode,#%001  wz    'wait for start bit on rx pin
                        test    rxmask,ina      wc
        if_z_eq_c       jmp     #receive

                        mov     rxbits,#9             'ready to receive byte
                        mov     rxcnt,bitticks
                        shr     rxcnt,#1              'half a bit tick
                        add     rxcnt,cnt             '+ the current clock             

:bit                    add     rxcnt,bitticks        'ready for the middle of the bit period

:wait                   jmpret  rxcode,txcode         'run a chuck of transmit code, then return

                        mov     t1,rxcnt              'check if bit receive period done
                        sub     t1,cnt
                        cmps    t1,#0           wc
        if_nc           jmp     #:wait

                        test    rxmask,ina      wc    'receive bit on rx pin into carry
                        rcr     rxdata,#1             'shift carry into receiver
                        djnz    rxbits,#:bit          'go get another bit till done

                        test    rxtxmode,#%001  wz    'find out if rx is inverted
        if_z_ne_c       jmp     #receive              'abort if no stop bit

                        shr     rxdata,#32-9          'justify and trim received byte
                        and     rxdata,#$FF
        if_nz           xor     rxdata,#$FF           'if rx inverted, invert byte

                        mov     t1, rcv_state
                        add     t1, #dispatch
                        jmp     t1

dispatch                jmp     #do_soh
                        jmp     #do_pktn
                        jmp     #do_type
                        jmp     #do_len_hi
                        jmp     #do_len_lo
                        jmp     #do_chk
                        jmp     #do_data
                        jmp     #do_crc_hi
                        jmp     #do_crc_lo

do_soh                  cmp     rxdata, #SOH wz
              if_z      mov     rcv_state, #STATE_PKTN
                        jmp     #receive              'byte done, receive next byte

do_pktn                 mov     rcv_chk, rxdata
                        mov     rcv_state, #STATE_TYPE
                        jmp     #receive              'byte done, receive next byte

do_type                 mov     rcv_type, rxdata
                        add     rcv_chk, rxdata
                        mov     rcv_state, #STATE_LEN_HI
                        jmp     #receive              'byte done, receive next byte

do_len_hi               mov     rcv_length, rxdata
                        shl     rcv_length, #8
                        add     rcv_chk, rxdata
                        mov     rcv_state, #STATE_LEN_LO
                        jmp     #receive              'byte done, receive next byte

do_len_lo               or      rcv_length, rxdata
                        add     rcv_chk, rxdata
                        mov     rcv_state, #STATE_CHK
                        jmp     #receive              'byte done, receive next byte

do_chk                  and     rcv_chk, #$ff
                        cmp     rxdata, rcv_chk wz
              if_nz     jmp     #send_nak
                        cmp     rcv_length, rcv_max wz, wc
              if_a      jmp     #send_nak
                        mov     crc, #0
                        mov     rcv_cnt, rcv_length wz
              if_z      mov     rcv_state, #STATE_CRC_HI
              if_nz     mov     rcv_state, #STATE_DATA
                        mov     rcv_ptr, rcv_buf1
                        jmp     #receive              'byte done, receive next byte

do_data                 call    #updcrc               'update the crc
                        wrbyte  rxdata, rcv_ptr
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
:wait                   jmpret  rxcode,txcode
                        rdlong  t1, pkt_status_ptr wz
              if_nz     jmp     #:wait
                        wrlong  rcv_type, pkt_type_ptr
                        wrlong  rcv_length, pkt_length_ptr
                        wrlong  rcv_buf1, pkt_buffer_ptr
                        mov     t1, #STATUS_OK
                        wrlong  t1, pkt_status_ptr
                        mov     t1, rcv_buf1
                        mov     rcv_buf1, rcv_buf2
                        mov     rcv_buf2, t1
                        mov     t1, #STATUS_OK
                        mov     sndbyte, #ACK
send_ack_nak            wrlong  t1, pkt_status_ptr
:wait                   jmpret  rxcode,txcode         'run a chunk of transmit code, then return
                        call    #send_byte
              if_z      jmp     #:wait
                        mov     rcv_state, #STATE_SOH
                        jmp     #receive              'byte done, receive next byte

send_nak                mov     t1, #STATUS_ERROR
                        mov     sndbyte, #NAK
                        jmp     #send_ack_nak

'
' Transmit
'
transmit                jmpret  txcode,rxcode         'run a chunk of mailbox code, then return

                        cmp     tx_head,tx_tail wz
        if_z            jmp     #transmit

                        add     tx_tail,txbuff        'get byte and inc tail
                        rdbyte  txdata,tx_tail
                        sub     tx_tail,txbuff
                        add     tx_tail,#1
                        and     tx_tail,tx_buffer_mask

                        or      txdata,#$100          'or in a stop bit
                        shl     txdata,#2
                        or      txdata,#1             'or in a idle line state and a start bit
                        mov     txbits,#11
                        mov     txcnt,cnt

:bit                    test    rxtxmode,#%100  wz    'output bit on tx pin according to mode
                        test    rxtxmode,#%010  wc
        if_z_and_c      xor     txdata,#1
                        shr     txdata,#1       wc
        if_z            muxc    outa,txmask        
        if_nz           muxnc   dira,txmask
                        add     txcnt,bitticks        'ready next cnt

:wait                   jmpret  txcode,rxcode         'run a chunk of mailbox code, then return

                        mov     t1,txcnt              'check if bit transmit period done
                        sub     t1,cnt
                        cmps    t1,#0           wc
        if_nc           jmp     #:wait

                        djnz    txbits,#:bit          'another bit to transmit?

                        jmp     #transmit             'byte done, transmit next byte

' send the byte in sndbyte
send_byte               mov     t1,tx_head            'check for head+1 <> tail
                        add     t1,#1
                        and     t1,tx_buffer_mask
                        cmp     t1,tx_tail wz
        if_z            jmp     #send_byte_ret
                        add     tx_head,txbuff        'put byte and inc head
                        wrbyte  sndbyte,tx_head
						mov		tx_head,t1
send_byte_ret           ret

'PRI updcrc(crc, data)
'  return (word[@crctab][(crc >> 8) & $ff] ^ (crc << 8) ^ data) & $ffff

updcrc                  mov     t1, crc
                        test    t1, #$100 wz
                        shr     t1, #9
                        add     t1, #crctab
                        movs    :load, t1
                        shl     crc, #8
:load                   mov     t1, 0-0
              if_nz     shr     t1, #16
                        xor     crc, t1
                        xor     crc, rxdata
                        and     crc, word_mask
updcrc_ret              ret

'
'
' Initialized data
'
'
zero                    long    0
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

'
' Uninitialized data
'
t1                      res     1
t2                      res     1

rxtxmode                res     1
bitticks                res     1

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

tx_buffer_mask          res     1

rcv_buf1                res     1
rcv_buf2                res     1
rcv_state               res     1
rcv_type                res     1
rcv_length              res     1  'packet length
rcv_chk                 res     1  'header checksum
rcv_max                 res     1  'maximum packet data length
rcv_ptr                 res     1  'data buffer pointer
rcv_cnt                 res     1  'data buffer count

pkt_type_ptr            res     1
pkt_buffer_ptr          res     1
pkt_length_ptr          res     1
pkt_status_ptr          res     1

crc                     res     1
sndbyte                 res     1

                            fit     496


